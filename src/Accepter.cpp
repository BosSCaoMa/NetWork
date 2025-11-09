#include "Accepter.h"
#include "socket.h"
#include "Channel.h"
#include "InetAddress.h"

#ifndef __linux__
#include "EpollCompat.h"
#endif

Accepter::Accepter(EventLoop *loop, const InetAddress &listenAddr)
: listening_(false),
  loop_(loop),
#ifdef __linux__
  acceptSocket_(::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP)),
#else
  acceptSocket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)),
#endif
  acceptChannel_{loop, acceptSocket_.fd()}
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Accepter::handleRead, this)); 
}

Accepter::~Accepter()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Accepter::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading(); // acceptChannel_注册至Poller !重要
}

void Accepter::handleRead()
{
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr); // 回调用户注册的处理新连接的函数
        } else {
            ::close(connfd); // 如果没有注册回调函数，直接关闭新连接
        }
    } else {
        // Handle error appropriately (e.g., log the error)
    }
}