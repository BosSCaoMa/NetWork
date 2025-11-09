#include "Thread.h"
#include <semaphore.h>
#include "CurrentThread.h"
#ifndef __linux__
#include "EpollCompat.h"
#endif
Thread::Thread(ThreadFunc func, const std::string &name)
: started_(false),
  joined_(false),
  thread_(nullptr),
  tid_(0),
  func_(std::move(func)),
  name_(name)
{
    setDefaultName();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        name_ = "Thread" + std::to_string(num);
    }
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        thread_->detach(); // 如果线程已经启动但没有被 join，则分离线程，避免资源泄漏
    }
}

void Thread::start() // 线程启动封装
{
    started_ = true;

    // 定义一个 信号量（semaphore） 变量，用于主线程与新线程之间的同步
    sem_t sem;
    sem_init(&sem, false, 0); // false 表示线程间共享，0 初始计数为 0,意味着 sem_wait() 会阻塞，直到另一方 sem_post()。

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]() {
        tid_ = CurrentThread::tid();                                        // 获取线程的tid值
        sem_post(&sem);
        func_();                                                            // 开启一个新线程 专门执行该线程函数
    }));

    sem_wait(&sem); // 主线程在这里 阻塞等待，直到新线程调用 sem_post(),这就保证了：主线程可以在 start() 返回前拿到子线程的 tid_，而不会出现未初始化的情况。
    sem_destroy(&sem); // 销毁信号量
}

void Thread::join()
{
    joined_ = true;
    if (thread_ && thread_->joinable()) {
        thread_->join(); // 等待线程结束，回收资源
    }
}