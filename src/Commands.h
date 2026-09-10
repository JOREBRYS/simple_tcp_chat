#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <array>

inline std::array<std::string, 10> commArray = {
    "PING",
    "HELP",
    "REGISTER",
    "LOGIN",
    "JOIN",
    "LEAVE",
    "MSG",
    "HISTORY",
    "LIST",
    "MENU"
};

enum class Command{
    PING = 0,
    HELP,
    REGISTER,
    LOGIN,
    JOIN,
    LEAVE,
    MSG,
    HISTORY,
    LIST,
    MENU
};

static int getCommandID(const std::string& comm){
    for(int i = 0; i < commArray.size(); i++){
        if(comm == commArray[i]){ return i; }
    }
    return -1;
}

#endif