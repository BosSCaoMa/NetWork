#pragma once
#include "nocopyable.h"
#include <vector>
#include <functional>
#include <string>
#include <memory>
class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : nocopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; } // 设置线程数量
    void start(const ThreadInitCallback &cb = ThreadInitCallback()); // 启动线程池

    EventLoop* getNextLoop(); // 获取下一个 EventLoop 对象指针，采用轮询方式分配
    std::vector<EventLoop*> getAllLoops(); // 获取所有 EventLoop 对象指针

    bool started() const { return started_; } // 线程池是否已启动
    const std::string& name() const { return name_; } // 获取线程池名称
private:
    EventLoop* baseLoop_; // 主线程的 EventLoop 对象指针
    std::string name_;    // 线程池名称
    bool started_;        // 线程池是否已启动
    int numThreads_;      // 线程数量
    int next_;           // 下一个被分配任务的线程索引
    std::vector<std::unique_ptr<EventLoopThread>> threads_; // 线程对象列表
    std::vector<EventLoop*> loops_; // 线程对应的 EventLoop 对象指针列表
};