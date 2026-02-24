#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Group.hpp"
#include "User.hpp"
#include "public.hpp"
//记录当前系统登录的用户信息
User g_currentUser;
//记录当前登录用户的好友列表信息
std::vector<User> g_currentUserFriendList;
//记录当前登录用户的群组列表信息
std::vector<Group> g_currentUserGroupList;
//显示当前登录成功用户的基本信息
void showCurrentUserData();

//接收线程
void readTaskHandler(int clientfd);
//获取系统时间（聊天信息需要添加时间信息）
std::string getCurrentTime();
//主聊天页面程序
void mainMenu();

int main(int argc, char *argv[]){
    if(argc < 3){
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == clientfd){
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(-1 == connect(clientfd, (sockaddr *)& server, sizeof(sockaddr_in))){
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    for(;;){
        std::cout << "==================="<<std::endl;
        std::cout << "1. login" <<std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "==================="<< std::endl;
        std::cout << "choice:";
        int choice = 0;
        std::cin>> choice;
        std::cin.get();

        switch (choice){
            case 1:{
                int id = 0;
                char pwd[50] = {0};
                std::cout << "userid:";
                std::cin>>id;
                std::cin.get();
                std::cout <<"userpassword:";
                std::cin.getline(pwd, 50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if(-1 == len){
                    std::cerr<<"send login msg error:" <<request <<std::endl;
                }else{
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(-1 == len){
                        std::cerr << "recv login response error"<< std::endl;
                    }else{
                        json responsejs = json::parse(buffer);
                        if(0 != responsejs["errno"].get<int>()){
                            std::cerr << responsejs["errmsg"] <<std::endl;
                        }else{
                            g_currentUser.setId(responsejs["id"].get<int>());
                            g_currentUser.setName(responsejs["name"]);

                            if(responsejs.contains("friends")){
                                std::vector<std::string> vec = responsejs["friends"];
                                for(std::string &str :vec){
                                    json js = json::parse(str);
                                    User user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    g_currentUserFriendList.push_back(user);
                                }
                            }

                            if(responsejs.contains("groups")){
                                std::vector<std::string> vec1 = responsejs["groups"];
                                for(std::string &groupstr : vec1){
                                    
                                }
                            }
                        }
                    }
                }
                
            }
            case 2:{
                char name[50] = {0};
                char pwd[50] = {0};
                std::cout << "username:";
                std::cin.getline(name, 50);
                std::cout << "userpassword:";
                std::cin.getline(pwd, 50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if(-1 == len){
                    std::cerr << "send reg msg error:"<<request << std::endl;
                }else{
                    char buffer[1024] = {0};
                    len = recv(clientfd, buffer, 1024, 0);
                    if(-1 == len){
                        std::cerr << "recv reg response error"<< std::endl;
                    }else{
                        json responsejs = json::parse(buffer);
                        if(0 != responsejs["errno"].get<int>()){
                            std::cerr<<name <<"is already exist, register error"<< std::endl;
                        }else{
                            std::cout << name << "register success ,userid is"<< responsejs["id"]
                                << ", do not forget it !"<< std::endl;
                        }
                    }
                }

            }
            break;
            case 3:
                close(clientfd);
                exit(0);
            default:
                std::cerr << "invalid input" << std::endl;
                break;
        }
    }
    return 0;
}

