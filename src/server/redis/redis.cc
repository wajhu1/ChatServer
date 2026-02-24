#include "redis.hpp"
#include <string>
#include <iostream>
    Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr){

    }
    Redis::~Redis(){
        if(_publish_context != nullptr){
            redisFree(_publish_context);
        }

        if(_subscribe_context != nullptr){
            redisFree(_subscribe_context);
        }
    }
    //连接Redis服务器
    bool Redis::connect(){
        _publish_context = redisConnect("127.0.0.1",6379);
        if(nullptr == _publish_context){
            std::cerr<< "connect redis failed"<<std::endl;
            return false;
        }

        _subscribe_context = redisConnect("127.0.0.1",6379);
        if(nullptr == _subscribe_context){
            std::cerr << "connect redis failed"<<std::endl;
            return false;
        }

        std::thread t([&](){
            observer_channel_message();
        });
        t.detach();
        std::cout<< "connect redis-server success"<<std::endl;

        return true;
    }
    //向redis指定的通道channel发布消息
    bool Redis::publish(int channel,std::string message){
        redisReply *reply = (redisReply *)redisCommand(_publish_context,"PUBLISH %d %s",channel,message.c_str());
        if(nullptr == reply){
            std::cerr << "publish command failed!"<<std::endl;
            return false;
        }
        freeReplyObject(reply);
        return true;
    }
    //向redis指定的通道subscribe订阅消息
    bool Redis::subscribe(int channel){
        //先缓存到本地
        if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"SUBSCRIBE %d",channel)){
            std::cerr<< "subscribe command failed!"<<std::endl;
            return false;
        }

        int done = 0;
        while(!done){
            //再发送到redis
            if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done)){
                std::cerr << "subscribe command failed!"<<std::endl;
                return false;
            }
        }

        return true;
    }
    //向redis指定的通道unsubscribe取消订阅消息
    bool Redis::unsubscribe(int channel){
       //先缓存到本地
        if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"UNSUBSCRIBE %d",channel)){
            std::cerr<< "unsubscribe command failed!"<<std::endl;
            return false;
        }

        int done = 0;
        while(!done){
            //再发送到redis
            if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done)){
                std::cerr << "unsubscribe command failed!"<<std::endl;
                return false;
            }
        }

        return true;
    }
    //在独立线程中接受订阅通道中的消息
    void Redis::observer_channel_message(){
        redisReply *reply = nullptr;
        while(REDIS_OK == redisGetReply(this->_subscribe_context,(void **)&reply)){
            if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr){
                _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
            }

            freeReplyObject(reply);
        }
        std::cerr<<">>>>>>>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<<"<<std::endl;
    }

    //初始化向业务层上报通道消息的回调对象
    void Redis::init_notify_handler(std::function<void(int, std::string)> fn){
        this->_notify_message_handler = fn;
    }
