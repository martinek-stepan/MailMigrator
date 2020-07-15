#pragma once
#include <string>

#include "utils.h"


class User
{
public:
    User(fs::path _username, std::string _domain)
        : domain(_domain)
        , username(_username)
    {
        usernameStr = username.filename().string();
    }

    fs::path username;
    std::string usernameStr;
    std::string domain;
    uint32 id = 0;
    uint32 root_folder = 0;
};
