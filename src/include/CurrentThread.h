#pragma once

#include <unistd.h>
#if defined(__linux__)
#include <sys/syscall.h>
#else
#include <EpollCompat.h>
#endif
namespace CurrentThread
{
    extern __thread int t_cachedTid; // 保存tid缓存 因为系统调用非常耗时 拿到tid后将其保存

    void cacheTid();

    inline int tid() // 内联函数只在当前文件中起作用
    {
        if (__builtin_expect(t_cachedTid == 0, 0)) // __builtin_expect 是一种底层优化 此语句意思是如果还未获取tid 进入if 通过cacheTid()系统调用获取tid
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}

/*
__thread  GCC的线程局部存储,意思是：每个线程都有自己独立的一份变量副本
extern __thread int t_cachedTid; 表示有一个线程局部变量 t_cachedTid，在别处定义，用来缓存线程 ID。
*/