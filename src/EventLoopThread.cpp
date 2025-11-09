#include "EventLoopThread.h"
#include "Thread.h"
#include "EventLoop.h"
#include <functional>
#include <memory>  

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_()
    , callback_(cb)
{
}

void EventLoopThread::threadFunc()
{
    EventLoop loop; // 每个线程都有一个独立的 EventLoop 对象

    if (callback_) {
        callback_(&loop); // 调用线程初始化回调函数
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop; // 将新创建的 EventLoop 对象的地址 &loop 保存到成员变量 loop_ 中。这个变量将告诉主线程：EventLoop 对象已经准备好了，可以开始使用。
        cond_.notify_one(); // 通知主线程 startLoop() 函数，EventLoop 已经创建完成
    }

    loop.loop(); // 启动事件循环

    {
        std::unique_lock<std::mutex> lock(mutex_); // 使用 std::unique_lock<std::mutex> 获取锁
        loop_ = nullptr; // 线程退出时，清空 EventLoop 指针
    }
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit(); // 退出事件循环
        thread_.join(); // 等待线程结束，回收资源
    }
}

EventLoop* EventLoopThread::startLoop() // 启动线程，创建并返回该线程中的 EventLoop 对象的指针
{
    thread_.start(); // 启动线程，执行 threadFunc()

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) { // 等待直到 loop_ 被设置
            cond_.wait(lock); // 释放锁并等待条件变量通知
        }
        loop = loop_; // 获取线程中的 EventLoop 对象指针
    }
    return loop; // 返回新创建的 EventLoop 对象的指针
}