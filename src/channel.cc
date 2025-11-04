/*
EPOLLIN      = 0x001    // 可读事件
EPOLLPRI     = 0x002    // 紧急数据可读(带外数据)
EPOLLOUT     = 0x004    // 可写事件
EPOLLERR     = 0x008    // 错误事件
EPOLLHUP     = 0x010    // 挂起事件
EPOLLRDHUP   = 0x2000   // 对端关闭连接或关闭写端
EPOLLET      = 0x80000000  // 边缘触发模式
EPOLLONESHOT = 0x40000000  // 一次性监听
*/
#if defined(__linux__)
#include <sys/epoll.h>
#else
#include "EpollCompat.h"
#endif
#include "channel.h"
#include "TimeStamp.h"
#include <iostream>
const int Channel::kNoneEvent = 0; //空事件
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI; //读事件
const int Channel::kWriteEvent = EPOLLOUT; //写事件

Channel::Channel(EventLoop *loop, int fd) : 
    fd_(fd),
    index_(-1),
    events_(0),
    revents_(0),
    loop_(loop),
    tied_(false) {}

Channel::~Channel() {} // 析构不 close 是因为 Channel 只是事件层适配器，不负责底层 fd 资源的所有权与释放，遵循单一职责与避免双重关闭。
