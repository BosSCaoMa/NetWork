#pragma once
#include <string>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

/*
    类作用：封装addr_结构体，提供IP和端口的转换接口
*/
class InetAddress{
public:
    explicit InetAddress(uint16_t port, std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {};

    const sockaddr_in& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const sockaddr_in& addr) { addr_ = addr; }

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;
private:
    sockaddr_in addr_;
};
