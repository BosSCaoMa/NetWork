# 模块设计

最佳的设计方法是从顶层到底层的设计和实现的结合。即进行自顶向下的规划，但在实现时根据需要从底层逐步深入

channel

tcpserver

poller

tcpconne

## accept
Acceptor 主要负责服务器监听套接字的创建、绑定、监听，以及新连接到来时的处理。它的典型用法如下：

在 TcpServer 中创建 Acceptor 对象
TcpServer 构造时会创建一个 Acceptor，传入监听端口和 EventLoop。

Acceptor 负责监听新连接
Acceptor 内部会创建一个监听套接字（socket），并注册到 EventLoop 的 Channel 上，关注读事件（即有新连接到来）。

新连接到来时，Acceptor 通过回调通知 TcpServer
当有新连接，Acceptor 的 Channel 触发读事件，调用 Acceptor 的 handleRead 方法，进而调用 TcpServer 注册的回调（newConnection），由 TcpServer 创建 TcpConnection 对象，管理这个新连接。

流程简述

TcpServer 创建 Acceptor
Acceptor 监听端口，注册读事件
有新连接时，Acceptor 触发回调，通知 TcpServer
TcpServer 创建 TcpConnection，后续交给 EventLoop 管理
总结：
Acceptor 负责“接收新连接”，是服务器端连接管理的入口，和 TcpServer、EventLoop 配合，实现高效的多路复用服务器。
## EventLoop

