#include "Buffer.h"
#include <errno.h>
#if !defined(_WIN32)
#include <sys/uio.h>
#else
#include "EpollCompat.h"
struct iovec { void *iov_base; size_t iov_len; };
#endif
#include <unistd.h>
size_t Buffer::readFd(int fd, int *savedErrno)
{
    char extraBuf[65536]; // 64KB栈上缓冲区

    /*
    struct iovec {
        ptr_t iov_base; // iov_base指向的缓冲区存放的是readv所接收的数据或是writev将要发送的数据
        size_t iov_len; // iov_len在各种情况下分别确定了接收的最大长度以及实际写入的长度
    };
    */
    struct iovec vec[2];

    size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len  = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len  = sizeof(extraBuf);

    // 根据 writable 大小决定使用 1 或 2 个 iovec 
    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *savedErrno = errno;
        return 0;
    } else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extraBuf, n - writable);
    }
    return n;
}

size_t Buffer::writeFd(int fd, int *savedErrno)
{
    size_t n = ::write(fd, peek(), readableBytes());
    if (n < 0) {
        *savedErrno = errno;
        return 0;
    }
    return n;  
}
