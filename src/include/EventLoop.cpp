#include "EventLoop.h"
#include "EpollPoller.h"
#include "TimeStamp.h"
#include "Channel.h"
#include "EpollCompat.h"
#include <unistd.h>        // for ::close
#include <functional>
const int kPollTimeMs = 10000;
__thread EventLoop *t_loopInThisThread = nullptr; // 线程局部变量，指向当前线程的EventLoop对象

int CreateWakeupFd()
{
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
    return eventfd;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one); // 把 wakeupFd_ 里的数据读出来，清除可读状态，让下次还能继续唤醒
    if (n != sizeof one) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one); // 向 wakeupFd_ 写入数据，触发它的可读事件，从而唤醒阻塞在 epoll_wait 上的线程
    if (n != sizeof one) {
        // Handle error appropriately (e.g., throw exception, log error)
    }
}

EventLoop ::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(new EpollPoller(this)),
      wakeupFd_(CreateWakeupFd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    if (t_loopInThisThread) {
        // Handle error appropriately (e.g., throw exception, log error)
    } else {
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this)); // 设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->enableReading(); // 让wakeupChannel_监听读事件
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}
//-------------主要函数---循环的执行-----------begin
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        TimeStamp pollReturnTime = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel* channel : activeChannels_) {
            channel->handleEvent(pollReturnTime);
        }

        doPendingFunctors();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    /*
    * 如果没有 callingPendingFunctors_，新加的回调不会被立即执行，只能等到下一个事件循环
    * 有了后，发现正在回调。就会主动调用 wakeup()，唤醒 EventLoop，让它尽快处理新加的回调，不用等到下一个事件循环
    */
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_); // 交换以减少锁的持有时间
    }

    for (const auto& func : functors) {
        func();
    }

    callingPendingFunctors_ = false;
}
// -------------主要函数---------------end

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

// EventLoop的方法 => Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::quit()
{
    quit_ = true;

    if (!isInLoopThread())
    {
        wakeup();
    }
}