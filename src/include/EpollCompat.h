#ifndef EPOLL_COMPAT_H
#define EPOLL_COMPAT_H

// 统一的 epoll 兼容层：
// Linux 下直接使用 <sys/epoll.h>
// 非 Linux 平台仅提供最小宏/结构/函数桩，便于其他代码编译通过。
// 这些桩不具备真实 I/O 复用能力，运行时需替换为 select / poll / IOCP / Boost.Asio 等实现。
#ifndef __linux__
#include <cstdint>

// 常用事件宏（保持与 Linux 数值一致或兼容用法，仅用于条件判断）
#ifndef EPOLLIN
#define EPOLLIN        0x001
#endif
#ifndef EPOLLPRI
#define EPOLLPRI       0x002
#endif
#ifndef EPOLLOUT
#define EPOLLOUT       0x004
#endif
#ifndef EPOLLERR
#define EPOLLERR       0x008
#endif
#ifndef EPOLLHUP
#define EPOLLHUP       0x010
#endif
#ifndef EPOLLRDHUP
#define EPOLLRDHUP     0x2000
#endif
#ifndef EPOLLET
#define EPOLLET        0x80000000
#endif
#ifndef EPOLLONESHOT
#define EPOLLONESHOT   0x40000000
#endif
#ifndef EPOLL_CLOEXEC
#define EPOLL_CLOEXEC  0
#endif
// Fallback definitions if epoll operation macros are missing
#ifndef EPOLL_CTL_ADD
#define EPOLL_CTL_ADD 1
#endif
#ifndef EPOLL_CTL_DEL
#define EPOLL_CTL_DEL 2
#endif
#ifndef EPOLL_CTL_MOD
#define EPOLL_CTL_MOD 3
#endif
#ifndef EFD_NONBLOCK
#define EFD_NONBLOCK 0x800000
#endif
#ifndef EFD_CLOEXEC
#define EFD_CLOEXEC 0x8000000
#endif
#ifndef SYS_gettid
#define SYS_gettid 186
#endif
#ifndef SOCKLEN_T
#define socklen_t int
#endif
// 结构体与函数桩，仅保证能被编译链接，不提供实际功能。
union epoll_data { void* ptr; int fd; uint32_t u32; uint64_t u64; };
struct epoll_event { uint32_t events; epoll_data data; };

inline int epoll_create1(int) { return -1; }
inline int epoll_ctl(int, int, int, struct epoll_event*) { return -1; }
inline int epoll_wait(int, struct epoll_event*, int, int) { return 0; }
inline int eventfd(unsigned int, int) { return -1; }
inline int syscall(int, ...) { return -1; }
inline int readv(int, const struct iovec*, int) { return -1; }
inline int close(int) { return -1; }
inline int send(int, const void*, size_t) { return -1; }
struct sockaddr_in {};
#endif // !__linux__

#endif // EPOLL_COMPAT_H
