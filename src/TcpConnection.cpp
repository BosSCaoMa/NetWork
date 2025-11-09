#include "TcpConnection.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include <functional>
#include "TimeStamp.h"
#include "EpollCompat.h"
TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : socket_(new Socket(sockfd)),
      state_(kConnecting),
      loop_(loop),
      channel_(new Channel(loop, sockfd)),
      name_(name),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      inputBuffer_(),
      outputBuffer_(),
      highWaterMark_(64 * 1024 * 1024) // 默认64MB
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true); // 开启 TCP KeepAlive 选项
}

TcpConnection::~TcpConnection()
{
    // 资源会被 unique_ptr 自动释放
}

// 读是相对服务器而言的 当对端客户端有数据到达 服务器端检测到EPOLLIN 就会触发该fd上的回调 handleRead取读走对端发来的数据
void TcpConnection::handleRead(TimeStamp receiveTime)
{
    // 读取数据到 inputBuffer_
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) { // 检查当前是否注册了 EPOLLOUT事件（写就绪）
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno); // 核心写入 尝试将 outputBuffer_ 中的数据写入套接字
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting(); // 取消EPOLLOUT事件的注册。为什么？因为没数据了，不用再监听写事件，节省epoll资源。
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shuntdownInLoop();
                }
            }
        } else {
            // 处理写错误
        }
    } else {
        // 不应该调用这个函数
    }
}

void TcpConnection::shuntdownInLoop()
{
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::handleClose()
{
    state_ = kDisconnected; // fixed: directly set state since setState is undefined
    channel_->disableAll();
    TcpConnectionPtr conn(shared_from_this());
    connectionCallback_(conn); // 通常用于通知用户连接状态变化（包括连接建立和断开）
    closeCallback_(conn); // 执行关闭连接的回调 执行的是TcpServer::removeConnection回调方法
}

void TcpConnection::handleError()
{
    // 通过 getsockopt(fd, SOL_SOCKET, SO_ERROR, ...) 获取 socket 的具体错误码
}

void TcpConnection::send(const std::string &message)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            loop_->queueInLoop(std::bind(&TcpConnection::sendInLoop, shared_from_this(), message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message)
{
    if (state_ == kDisconnected) {
        return; // 连接已断开，不能发送数据
    }

    ssize_t nwrote = 0;
    size_t remaining = message.size();
    bool faultError = false;

    // 满足两个条件才会直写：当前未监听可写事件（说明之前没有积压待发数据） + 输出缓冲区为空。
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        ssize_t n = ::send(channel_->fd(), message.data(), message.size());
        if (n > 0) { // nwrote >= 0：成功写了一部分或全部
            remaining  = message.size() - n;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }  
        } else { // 处理写错误
            nwrote = 0;
            // EWOULDBLOCK/EAGAIN 在非阻塞 fd 上表示现在写不下，稍后再试，是“正常情况”。
            if (errno != EWOULDBLOCK) { // 是真错，记日志
                if (errno == EPIPE || errno == ECONNRESET) { // 对端已关闭或复位，置 faultError = true，后续不再缓存这批数据。
                    faultError = true;
                }
            }
            
        }
    }
    // 还有剩余数据没有写完，存入发送缓冲区 outputBuffer_, 并注册 channel 的写事件，确保能尽快发送剩余数据
    if (!faultError && remaining > 0) { // 只有在没有致命错误且还有未写完的数据时才走这块
        size_t oldLen = outputBuffer_.readableBytes(); // 追加前，发送缓冲区里尚未发出去的数据量。后面要用它判断“是否跨过高水位线”。
        if (oldLen + remaining >= highWaterMark_ 
            && highWaterMarkCallback_ // 意味着这次 append() 操作使缓冲区从“低于高水位线”跨到了“达到或超过”高水位线。
            && oldLen < highWaterMark_) { // 这就是所谓的“跨线事件”。只在这一瞬间触发回调一次，避免反复通知
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }

        outputBuffer_.append(message.data() + nwrote, remaining); // 把剩余数据添加到发送缓冲区
        if (!channel_->isWriting()) {
            channel_->enableWriting(); // 这里一定要注册channel的写事件 否则poller不会给channel通知epollout
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        loop_->runInLoop(std::bind(&TcpConnection::shuntdownInLoop, shared_from_this()));
    }
}

void TcpConnection::connectEstablished()
{
    state_ = kConnected;
    channel_->tie(shared_from_this());
    channel_->enableReading(); // 注册读事件
    connectionCallback_(shared_from_this()); // 执行用户注册的连接建立回调函数
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected) {
        state_ = kDisconnected;
        channel_->disableAll(); // 注销所有事件
        connectionCallback_(shared_from_this()); // 执行连接断开回调函数
    }
    channel_->remove(); // 从Poller中删除该Channel
}

void TcpConnection::sendFile(int filefd, size_t length, size_t count)
{
    // 后续实现
}

