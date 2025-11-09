#pragma once

#include "nocopyable.h"
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include "CurrentThread.h" 
class Channel;
class Poller;
/*
|----> CurrentThread
*/
using ChannelList = std::vector<Channel*>;
using Functor =  std::function<void()>;
// EventLoop就是负责实现“循环”，负责驱动“循环”的重要模块
class EventLoop : nocopyable {
public:
    
    EventLoop();
    ~EventLoop();

    void loop(); // 循环函数，开启事件循环
    void runInLoop(Functor cb); // 在当前loop中执行cb
    void queueInLoop(Functor cb); // 在当前loop中执行cb
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); } 

    void updateChannel(Channel *channel); // 更新Channel
    void removeChannel(Channel *channel); // 移除Channel
    bool hasChannel(Channel *channel);
    void quit();
private:
    ChannelList activeChannels_; // 活跃的Channel列表
    std::unique_ptr<Poller> poller_; // IO复用的具体实现对象

    std::atomic<bool> looping_;
    std::atomic<bool> quit_;

    std::mutex mutex_; // 保护pendingFunctors_的互斥锁
    std::atomic<bool> callingPendingFunctors_; // 标志当前是否有线程正在
    std::vector<Functor> pendingFunctors_; // 存储其他线程提交的回调操作

    int wakeupFd_; // 通过该成员唤醒subLoop处理Channel
    std::unique_ptr<Channel> wakeupChannel_; // 专门负责监听wakeupFd_的Channel
    const pid_t threadId_; // 记录当前EventLoop所属的线程ID

    void handleRead();
    void wakeup();
    void doPendingFunctors(); // 执行其他线程提交的回调操作
    
};