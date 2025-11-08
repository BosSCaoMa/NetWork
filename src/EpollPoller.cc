#if defined(__linux__)
#include <sys/epoll.h>
#else
#include "EpollCompat.h"
#endif
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "EpollPoller.h"

const int kNew = -1;    // 某个channel还没添加至Poller          // channel的成员index_初始化为-1
const int kAdded = 1;   // 某个channel已经添加至Poller
const int kDeleted = 2; // 某个channel已经从Poller删除

/*
* epoll_create1 是一个系统调用，返回一个新的 epoll 文件描述符。这个文件描述符将用于后续的 epoll 操作（如等待 I/O 事件等）
* EPOLL_CLOEXEC 是一个标志，表示在执行 exec() 系统调用时，会自动关闭 epollfd_，从而避免描述符泄漏到子进程。
*/
EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),  
      events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

/*
* epoll_wait：核心的系统调用，用于等待 I/O 事件。它会阻塞，直到至少有一个文件描述符上发生了事件，或者超时
*/
TimeStamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // 感兴趣的事件都是在epoll_ctl添加的
    int numEvents = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);

    if (numEvents > 0) {
        // 处理活跃的事件
        fillActiveChannel(numEvents, activeChannels);
        if (numEvents == events_.size()) {
            events_.resize(events_.size() * 2); // 扩容
        }
    }
}

void EpollPoller::fillActiveChannel(int numEvents, ChannelList *activeChannels)
{
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        else // index == kDeleted
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // channel已经在Poller中注册过了
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel); // 如果它不再需要监听任何事件，则从 epoll 中删除；
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 从Poller中删除channel
void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 更新epoll实例中的某个文件描述符对应的事件监听设置
void EpollPoller::update(int operation, Channel *channel)
{
    int fd = channel->fd();

    epoll_event event;
    memset(&event, 0, sizeof event);

    event.events = static_cast<uint32_t>(channel->events());
    event.data.fd = fd;
    event.data.ptr = channel; // 保存Channel指针，方便后续处理事件时获取对应的Channel对象

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
}
