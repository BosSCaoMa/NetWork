#ifndef POLLER_H
#define POLLER_H

#include "nocopyable.h"
#include "TimeStamp.h"
#include <unordered_map>
#include <memory>
#include <vector>
class EventLoop;
class Channel;

using ChannelList = std::vector<Channel*>;

class Poller : nocopyable {
public:
    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    virtual TimeStamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    // 判断参数channel是否在当前的Poller当中
    bool hasChannel(Channel *channel) const;

protected:
    std::unordered_map<int, Channel*> channels_; // fd到Channel的映射

    EventLoop* ownerLoop_; // 归属的事件循环
    /*父类的 private 变量会被子类继承（内存上存在于子类对象中），
    但子类无法直接访问（无论在子类内部还是外部）。
    只能通过父类提供的 public 或 protected 成员函数间接访问。*/
};


#endif  // POLLER_H