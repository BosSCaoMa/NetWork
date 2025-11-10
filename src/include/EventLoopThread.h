#pragma once

#include "nocopyable.h"
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>
class EventLoop;
class Thread;
class EventLoopThread : nocopyable { 
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop(); // 启动线程，创建并返回该线程中的 EventLoop 对象的指针
private:
    EventLoop *loop_; // 线程中的 EventLoop 对象指针
    bool exiting_; // 线程是否正在退出
    ThreadInitCallback callback_; // 线程初始化回调函数
    std::string name_; // 线程名称

    Thread thread_;
    std::mutex mutex_;             // 互斥锁
    std::condition_variable cond_; // 条件变量
    
    void threadFunc(); // 线程函数，在线程中运行 EventLoop
};
