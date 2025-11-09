#pragma once

#include "nocopyable.h"
#include <functional>
#include <thread>
#include <atomic>
#include <string>
#include <memory>
#ifndef __linux__
#include "EpollCompat.h"
#endif
class Thread : nocopyable {
public:
    using ThreadFunc = std::function<void()>; // 定义线程函数类型

    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();
    bool started() const { return started_; }
    pid_t tid() const { return tid_; } // 在 Linux 下，pid_t 是进程/线程ID的标准类型，线程创建后会通过系统调用（如 gettid() 或 pthread_self()）获取并赋值给 tid_
    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_; } // 返回已创建线程的数量

private:
    void setDefaultName();


    std::atomic<bool> started_;
    std::atomic<bool> joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;       // 在线程创建时再绑定，tid_ 用来唯一标识当前线程
    ThreadFunc func_;  // 回调函数
    std::string name_;
    static std::atomic<int> numCreated_;
};