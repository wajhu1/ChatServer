#pragma once

#include "Group.hpp"
#include <string>
#include <vector>

class GroupModel{
public:
    //create group
    bool createGroup(Group & group);
    //join group
    void addGroup(int userid, int groupid, std::string role);
    //query group info
    std::vector<Group> queryGroups(int userid);
    //use userid groupid query info
    std::vector<int> queryGroupUsers(int userid, int groupid);

};

