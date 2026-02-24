#pragma once
#include"GroupUser.hpp"
#include <string>
#include <vector>

class Group{
public:
    Group(int id = -1, std::string name = "", std::string desc = ""):_id(id), _name(name), _desc(desc){

    }

    void setId(int id) {_id = id;}
    void setName(std::string name){_name = name;}
    void setDesc(std::string desc) {_desc = desc;}

    int getId() {return _id;}
    std::string getName(){return _name;}
    std::string getDesc() {return _desc;}
    std::vector<GroupUser> &getUsers() {return _users;}

private:
    int _id;
    //组的名称
    std::string _name;
    //组的描述
    std::string _desc;
    //记录组的成员
    std::vector<GroupUser> _users;
};