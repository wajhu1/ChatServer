#include "ChatServer.hpp"
#include "json.hpp"
#include <functional>
#include <string>
#include <iostream>
#include "ChatService.hpp"
using json = nlohmann::json;


ChatServer::ChatServer(EventLoop *loop
                        ,const InetAddress &listenAddr
                        ,const string &nameArg)
    :_loop(loop), _server(loop, listenAddr, nameArg){
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this, std::placeholders::_1));

        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2, std::placeholders::_3));

        _server.setThreadNum(4);
    }


void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn){
    if(!conn->connected()){
        ChatService::getInstance().clientCloseException(conn);
        //std::cout << "exit errno"<<std::endl;
        conn->shutdown();
    }else{

    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn,
                            Buffer *buffer,
                            Timestamp time){
    std::string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);
    auto msgHandler = ChatService::getInstance().getHandler(js["msgid"].get<int>());
    msgHandler(conn,js,time);
}


