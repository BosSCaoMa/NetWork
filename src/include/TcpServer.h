#pragma once

#include <functional>
#include <string>
#include "CallBacks.h"
#include <atomic>
class InetAddress;
class EventLoop;
class EventLoopThreadPool;
class TcpServer{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option{
        kNoReusePort,
        kReusePort,
    };
TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option = kNoReusePort);
    ~TcpServer();

    void setThreadNum(int numThreads); // 设置底层subloop的数量，即工作线程的数量
    void start(); // 启动服务器

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
	using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

	ConnectionCallback connectionCallback_;       //有新连接时的回调
    MessageCallback messageCallback_;             // 有读写事件发生时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成后的回调
    ThreadInitCallback threadInitCallback_; // loop线程初始化的回调

	std::atomic<int> started_; // 服务器是否已启动
	
    int numThreads_; // 工作线程的数量
	std::string name_; // 服务器名称

	EventLoop* loop_; // baseLoop，用户定义的loop
	std::shared_ptr<EventLoopThreadPool> threadPool_; // 事件循环线程池
	std::unique_ptr<Accepter> accepter_; // 连接接受器

	const std::string ipPort_;
    const std::string name_;
	int nextConnId_; // 下一个连接的ID
	ConnectionMap connections_; // 保存所有的连接

	void newConnection(int sockfd, const InetAddress& peerAddr); // 有新连接时的回调
	void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
};