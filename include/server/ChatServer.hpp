#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer{
public:
    // 构造函数
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 启动服务器
    void start();
private:
    // 处理消息
    void onMessage(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp time);
    // 处理连接
    void onConnection(const TcpConnectionPtr &conn);
    TcpServer _server;//创建一个服务器对象
    EventLoop *_loop;//创建一个事件循环
};