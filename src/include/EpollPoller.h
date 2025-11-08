#pragma once

#include "nocopyable.h"
#include "Poller.h"
#include "Channel.h"
#include "TimeStamp.h"
#include <vector>

/**
 * epoll的使用:
 * 1. epoll_create
 * 2. epoll_ctl (add, mod, del)
 * 3. epoll_wait
 **/

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller();

    TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override; // 调用一次Poller::poll方法它就能给你返回事件监听器的监听结果
    void fillActiveChannel(int numEvents, ChannelList *activeChannels);
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16; // 初始事件列表大小
    int epollfd_; // epoll文件描述符
    std::vector<epoll_event> events_; // 用于存储epoll事件

    // 省略 void fillActiveChannels(int numEvents, ChannelList* activeChannels);
    void update(int operation, Channel* channel);
};