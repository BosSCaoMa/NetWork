#pragma once
#include "nocopyable.h"
#include <memory>
#include <string>
#include <functional>
#include "CallBacks.h"
#include "Buffer.h"
class Buffer;
class EventLoop;
class Channel;
class InetAddress;
class Socket;
class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection> {
    // 如果你只用裸指针调用 shared_from_this() 会直接崩掉，但继承它就能正确工作
    // enable_shared_from_this允许在内部安全获取 shared_ptr 到自己
public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    // 获得私有变量
    const std::string& getName() const { return name_; }
    const std::string& getName() const { return name_.c_str(); }
    const InetAddress& getLocalAddr() { return localAddr_; };
    const InetAddress& getPeerAddr() { return peerAddr_; };
    bool connected() const { return state_ == kConnected; };

    // 发送数据
    void send(const std::string& message);
    void sendFile(int filefd, size_t length, size_t count); // 新增的零拷贝发送函数

    // 设置各种回调函数
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
        highWaterMarkCallback_ = cb; 
        highWaterMark_ = highWaterMark;
    }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    // 连接相关
    void shutdown(); // 关闭半连接
    void connectEstablished(); // 连接建立
    void connectDestroyed(); // 连接销毁

    EventLoop* getLoop() { return loop_; }

    const std::string &name() const { return name_; }
private:
    enum StateE {
        kDisconnected, // 已经断开连接
        kConnecting,   // 正在连接
        kConnected,    // 已连接
        kDisconnecting // 正在断开连接
    };

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    StateE state_;
    EventLoop* loop_;
    const std::string name_;

    const InetAddress peerAddr_;
    const InetAddress localAddr_;
    // 回调函数
    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_; // 高水位回调
    CloseCallback closeCallback_; // 关闭连接的回调
    size_t highWaterMark_; // 高水位阈值

    void handleRead(TimeStamp receiveTime); // 读事件回调
    void handleWrite(); // 写事件回调
    void handleClose(); // 关闭事件回调
    void handleError(); // 错误事件回调

    void shuntdownInLoop(); // 在loop中关闭连接

    void sendInLoop(const std::string& message); // 在loop中发送数据
};