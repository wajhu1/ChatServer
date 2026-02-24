#pragma once
#include "User.hpp"

class GroupUser : public User{
public:
    void setRole(std::string role){ _role = role;}
    std::string getRole() {return _role;}
private:
    std::string _role;

};