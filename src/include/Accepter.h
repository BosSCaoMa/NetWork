#pragma once

#include "nocopyable.h"
#include <functional>

/*
|----> inetAddress
|----> socket
*/
class InetAddress;
class EventLoop;
class Socket;
class Channel;

/*
Q:没有定义Cpp文件时，前向声明似乎会失效，提示“不允许使用不完整的类型”
A:你的前向声明 没有“失效”，它其实是起作用的。
这里的问题是：作为类成员时，需要知道 Channel 的完整大小，但前向声明并不能提供大小信息。
为什么 EventLoop* loop_ 就可以？因为指针类型的大小是固定的（通常 8 字节）。
如何解决：在 Accepter.cpp 中包含 Channel.h 和 InetAddress.h 的定义。
*/

class Accepter : nocopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Accepter(EventLoop* loop, const InetAddress& listenAddr);
    ~Accepter();

    // TcpServer 会调用这个函数，注册用户处理新连接的回调函数
    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectionCallback_ = cb; }
    void listen();
    bool listening() const { return listening_; }
private:
    bool listening_;
    EventLoop* loop_; // 主事件循环
    Socket acceptSocket_; // 监听套接字
    Channel acceptChannel_; // 监听套接字对应的Channel
    NewConnectionCallback newConnectionCallback_;

    void handleRead(); // 处理新用户连接事件
};
