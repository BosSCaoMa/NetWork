#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop), name_(nameArg), started_(false), numThreads_(0), next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        std::string threadName = name_ + std::to_string(i);
        auto thread = std::make_unique<EventLoopThread>(cb, threadName);
        EventLoop *loop = thread->startLoop();
        loops_.push_back(loop);
        threads_.push_back(std::move(thread));
    }

    if (numThreads_ == 0 && cb) {// 如果线程数量为 0，说明整个服务器只使用 主线程的 baseLoop_ 来执行事件循环
        cb(baseLoop_); // 仍然执行回调函数 cb(baseLoop_)，让用户在单线程模式下做初始化操作
    }
}

// 负载均衡算法，可改为一致性哈希等更复杂的算法
EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }

    return loop;
}


std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}