#pragma once
#include <unordered_map>
#include "json.hpp"
#include <muduo/net/TcpServer.h>
#include <functional>
#include "redis.hpp"
#include "UserModel.hpp"
#include "OfflineMsgModel.hpp"
#include <mutex>
#include "FriendModel.hpp"
#include "GroupModel.hpp"

using namespace muduo;
using namespace muduo::net;

using json = nlohmann::json;

using msgHandler = std::function<void(const TcpConnectionPtr&conn, json &js, Timestamp time)>;

//聊天服务器业务类
class ChatService{
public:
    static ChatService& getInstance();
    //处理登录业务
    void login(const TcpConnectionPtr &conn, json& js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js,Timestamp time);
    //获取消息对应的处理器
    msgHandler getHandler(int msgid);

    //服务器异常 业务重置方法
    void reset();

    //客户端异常退出
    void clientCloseException(const TcpConnectionPtr&conn);

    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    void handleRedisSubscribeMessage(int userid, std::string msg);
private:
    ChatService();

    //存储消息id和其对应的业务处理方法
    std::unordered_map<int, msgHandler> _msgHandlerMap;
    //数据操作对象
    UserModel _userModel;

    //存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> _userConnMap;

    //定义互斥锁
    std::mutex _connMutex;

    //
    OfflineMsgModel _offlineMsgModel;

    FriendModel _friendModel;

    GroupModel _groupModel;

    Redis _redis;
};