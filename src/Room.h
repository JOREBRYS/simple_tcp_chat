#ifndef ROOMS_H
#define ROOMS_H

//#include "Database.h"
#include "Message.h"
//#include "Session.h"

#include <string>
#include <vector>
#include <memory>

struct RoomConf{
    unsigned int id_;
    std::string name_;

    RoomConf() {};
    RoomConf(unsigned int roomID, const std::string& roomname):
        id_(roomID), name_(roomname) {}
};

class Session;
class Database;

class Room{
    private:
    RoomConf info_;
    std::vector<Message> history_;
    std::vector<std::weak_ptr<Session>> participants_;
    Database& db_;

    public:
    Room(const std::string& name, int id, Database& db) : info_(id, name), db_(db) {};
    Room(const RoomConf& info, Database& db) : info_(info), db_(db) {}

    void add_participant(std::shared_ptr<Session> session);
    void remove_participant(std::shared_ptr<Session> session);
    void broadcast(const std::string& text, const std::shared_ptr<Session> sender);
    void add_message(const Message& message);
    std::vector<Message> get_history(int N);

    unsigned int getID() const {
        return info_.id_;
    }

    std::string getName() const {
        return info_.name_;
    }
};

#endif