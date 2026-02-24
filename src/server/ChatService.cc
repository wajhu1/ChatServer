#include "ChatService.hpp"
#include "public.hpp"
#include <string>
#include <iostream>
#include <muduo/base/Logging.h>
using namespace muduo;

ChatService& ChatService::getInstance(){
    static ChatService service;
    return service;
}

ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});

    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat,this , std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 )});

    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 )});

    if(_redis.connect()){
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,std::placeholders::_1,std::placeholders::_2));
    }
}

//处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LOG_INFO <<"do login service!!!";
    int id = js["id"].get<int>();
    std::string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id &&user.getPassword() == pwd){
        if(user.getState() == "online"){
            //已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 2;
            response["errmsg"] = "该账号已经登录";

            conn->send(response.dump());
        }else{
            {
                //登录成功，记录用户连接信息
                std::lock_guard<std::mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});
            }

            _redis.subscribe(id);

            //登录成功，更新用户状态信息 state offline->online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            std::vector<std::string> vec = _offlineMsgModel.query(user.getId());
            if(!vec.empty()){
                response["offlinemsg"] = vec;
                _offlineMsgModel.remove(user.getId());
            }

            std::vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                std::vector<std::string> vec2;
                for(User &user : userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            conn->send(response.dump());
        }
    }else{
        //用户不存在或者密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["error"] = 1;
        conn->send(response.dump());
    }
}
//处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LOG_INFO << "do reg service!!!";
    std::string name = js["name"];
    std::string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);
    
    bool state = _userModel.insert(user);
    if(state){
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }else{
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 1;
        response["id"] = user.getId();
        conn->send(response.dump());
    }

}
msgHandler ChatService::getHandler(int msgid){
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        return [msgid](const TcpConnectionPtr&conn, json &js, Timestamp time){
            LOG_ERROR << "msgid: "<< msgid <<"can not find handler";
        };
    }else{
        return _msgHandlerMap[msgid];
    }
}


void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        
        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it){
            if(it->second == conn){
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    _redis.unsubscribe(user.getId());

    if(user.getId() != -1){
        user.setState("offline");
        _userModel.updateState(user);

        std::cout << "success" <<std::endl;
        return ;
    }
    std::cout << "failed" <<std::endl;
    return;
}

//1对1聊天通信
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int toid = js["to"].get<int>();
    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end()){
            //在线就直接发送消息
            it->second->send(js.dump());
            return;
        }
    }

    User user = _userModel.query(toid);
    if(user.getState() == "online"){
        _redis.publish(toid, js.dump());
    }



    //不在线
    _offlineMsgModel.insert(toid, js.dump());
}



void ChatService::reset(){
    _userModel.resetState();
}


void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    _friendModel.insert(userid, friendid);
}



void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];

    Group group(-1, name, desc);
    if(_groupModel.createGroup(group)){
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);
    std::lock_guard<std::mutex> lock(_connMutex);
    for(int id : useridVec){

        auto it = _userConnMap.find(id);
        if( it != _userConnMap.end()){
            it->second->send(js.dump());
        }else{
            User user = _userModel.query(id);
            if(user.getState() == "online"){
                _redis.publish(id,js.dump());
            }else{
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

void ChatService::handleRedisSubscribeMessage(int userid, std::string msg){

    std::lock_guard<std::mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end()){
        it->second->send(msg);
        return ;
    }

    _offlineMsgModel.insert(userid, msg);
}
