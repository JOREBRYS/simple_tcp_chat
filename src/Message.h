#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <chrono>

struct Message{
    int userID;
    std::string username;
    std::string text;
    std::chrono::system_clock::time_point timestamp;

    Message() {}
    Message(int userid, std::string username, std::string text,
        std::chrono::system_clock::time_point timestamp):
        userID(userid), username(username), text(text), timestamp(timestamp) {}
};

#endif 