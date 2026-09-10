#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>
#include "Message.h"
#include "Room.h"
#include <vector>


class Database{
    private:
    sqlite3* db_;

    std::string hashPassword(const std::string& password);
    bool createTables();

    public:
    Database();
    ~Database();

    int registerUser(const std::string& userName, const std::string& password);
    int loginUser(const std::string& userName, const std::string& password);
    std::vector<RoomConf> getRoomsList(int userID);
    void joinRoom(int userID, int roomID);
    void leaveRoom(int userID, int roomID);

    int createRoom(const std::string& roomname);
    int getRoomID(const std::string& roomname);
    bool deleteRoom(const int roomID);
    bool saveMessage(const Message& message, int roomID);
    std::vector<Message> getHistory(int roomID, int N);
};

#endif