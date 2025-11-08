#include "include/InetAddress.h"
#include <cstring>
#include <sstream>
#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());
}

std::string InetAddress::toIp() const
{
    char buf[INET_ADDRSTRLEN] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    /*
    const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
        af: 地址族，IPv4 使用 AF_INET。
        src: 指向要转换的网络字节序地址的指针，通常是 struct in_addr 类型的 sin_addr。
        dst: 存放转换后字符串的缓冲区，应该足够大。
        size: 缓冲区的大小，通常为 INET_ADDRSTRLEN（IPv4 地址 16 字节）
    */
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = ::strlen(buf);
    uint16_t port = ::ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}
