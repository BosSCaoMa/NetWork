#pragma once

#include <vector>
#include <string>
class Buffer {
public:
    static const size_t kCheapPrepend = 8; // 初始预留的prependable空间大小
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {}

    size_t readableBytes() const { return writerIndex_ - readerIndex_; } // 可读数据的大小
    size_t writableBytes() const { return buffer_.size() - writerIndex_; } // 可写数据的大小
    size_t prependableBytes() const { return readerIndex_; }

    char *peek() { return begin() + readerIndex_; } // 返回可读数据的起始位置
    void retrieve(size_t len) {
        if (len < readableBytes()) { // 只移动读取索引
            readerIndex_ += len; 
        } else { // 全部数据被读取
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend;
        }
    }

    std::string retrieveAsString() {
        size_t len = readableBytes();
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    // 把[data, data+len]内存上的数据添加到writable缓冲区当中
    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, begin() + writerIndex_);
        writerIndex_ += len;
    }

    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len); // 扩容
        }
    }

    size_t readFd(int fd, int* savedErrno); // 从fd上读取数据，存入缓冲区
    size_t writeFd(int fd, int* savedErrno); // 将缓冲区中的数据写入fd
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    char *begin() { return buffer_.data(); }
    void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) { // 也就是说 len > xxx前面剩余的空间 + writer的部分
            buffer_.resize(writerIndex_ + len); // 直接扩容
        } else { // 将数据搬移到前面，腾出空间
            size_t readable = readableBytes();
            // 将当前缓冲区中从readerIndex_到writerIndex_的数据
            // 拷贝到缓冲区起始位置kCheapPrepend处，以便腾出更多的可写空间
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
};