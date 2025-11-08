#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include "nocopyable.h"
#include <memory>
class EventLoop;
class TimeStamp;

class Channel : nocopyable {
public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    using ReadEventCallback = std::function<void(TimeStamp)>;
    using EventCallback =  std::function<void()>;
    void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }
    void setEvents(int events) { revents_ = events; }

    void handleEvent(TimeStamp receiveTime); // 根据发生的事件调用相应的回调函数，时间戳参数表示事件发生的时间，用来记录日志或进行时间相关的处理
    // 防止当Channel被手动remove掉 Channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    EventLoop *ownerLoop() { return loop_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update(); }
    
    void remove();  // 从Poller中删除自己
private:
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);  // 确保在调用回调函数时，Channel对象仍然存在, tied_为true时调用

    int fd_;
    int index_;  // 用于标识Channel在EventLoop中的位置
    int events_;   // 感兴趣的事件
    int revents_;  // 真实发生的事件
    EventLoop* loop_;  // 归属的事件循环

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
};

#endif // CHANNEL_H