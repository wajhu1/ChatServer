#include "ChatServer.hpp"
#include "ChatService.hpp"
#include <signal.h>
#include<iostream>
//
void resetHandler(int){
    ChatService::getInstance().reset();
    exit(0);
}

int main(){

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);

    ChatServer server(&loop, addr, "chatserver");

    server.start();

    loop.loop();

    return 0;
}