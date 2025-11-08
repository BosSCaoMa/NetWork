#pragma once

#include "nocopyable.h"

class InetAddress;
class Socket : nocopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peerAddr);

    void shutdownWrite(); // 关闭写端,表示不再发送数据，但还能接收数据。常用于 TCP 连接的半关闭
    
    void setReuseAddr(bool on); // 允许重用本地地址
    void setReusePort(bool on); // 允许重用本地端口
    void setKeepAlive(bool on); // 开启 TCP KeepAlive 选项
    void setTcpNoDelay(bool on); // 禁用 Nagle 算法
private:
    const int sockfd_;
};