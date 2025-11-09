#include "socket.h"
#include <unistd.h>
#include "InetAddress.h"
Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &addr)
{
    #ifdef _WIN32
    ::bind(sockfd_, reinterpret_cast<const struct sockaddr*>(&addr.getSockAddrInet()), sizeof(struct sockaddr_in));
    #else
    if (0 != ::bind(sockfd_, (sockaddr *)addr.getSockAddrInet(), sizeof(sockaddr_in))) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
    #endif
}

void Socket::listen()
{
    if (0 != ::listen(sockfd_, 1024)) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
}

int Socket::accept(InetAddress *peerAddr)
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept(sockfd_, (sockaddr *)&addr, &addrlen);
    if (connfd >= 0) {
        peerAddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    #ifdef _WIN32
    ::shutdown(sockfd_, SD_SEND);
    #else
    if (0 != ::shutdown(sockfd_, SHUT_WR)) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
    #endif
}

void Socket::setTcpNoDelay(bool on)
{
    // TCP_NODELAY 用于禁用 Nagle 算法。
    // Nagle 算法用于减少网络上传输的小数据包数量。
    // 将 TCP_NODELAY 设置为 1 可以禁用该算法，允许小数据包立即发送。
    int optval = on ? 1 : 0;
    #ifdef _WIN32
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, "1", sizeof(optval));
    #else
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    #endif
}

void Socket::setReuseAddr(bool on)
{
    // SO_REUSEADDR 允许一个套接字强制绑定到一个已被其他套接字使用的端口。
    // 这对于需要重启并绑定到相同端口的服务器应用程序非常有用。
    int optval = on ? 1 : 0;
    #ifdef _WIN32
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, "1", sizeof(optval));
    #else
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    #endif
}

void Socket::setReusePort(bool on)
{
    // SO_REUSEPORT 允许同一主机上的多个套接字绑定到相同的端口号。
    // 这对于在多个线程或进程之间负载均衡传入连接非常有用。
    int optval = on ? 1 : 0;
    #ifdef _WIN32
    ::setsockopt(sockfd_, SOL_SOCKET, 1, "1", sizeof(optval));
    #else
    if (0 != ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
    #endif
}

void Socket::setKeepAlive(bool on)
{
    // SO_KEEPALIVE 启用在已连接的套接字上定期传输消息。
    // 如果另一端没有响应，则认为连接已断开并关闭。
    // 这对于检测网络中失效的对等方非常有用。
    int optval = on ? 1 : 0;
    #ifdef _WIN32
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, "1", sizeof(optval));
    #else
    if (0 != ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval))) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
    #endif  
}