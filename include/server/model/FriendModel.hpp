#pragma once
#include <vector>
#include <User.hpp>

class FriendModel{
public:
    //add friend
    void insert(int userid, int friendid);

    //return friendid table
    std::vector<User> query(int userid);
};