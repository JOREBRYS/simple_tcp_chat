#include "Server.h"

std::shared_ptr<Room> Server::get_or_create_room(const std::string& roomname){
    auto it = rooms_.find(roomname);
    if(it != rooms_.end()){
        return it->second;
    }

    int id = db_.getRoomID(roomname);
    if(id != -1){
        auto room = std::make_shared<Room>(roomname, id, db_);
        rooms_[roomname] = room;
        return room;
    }
    else{
        int id = db_.createRoom(roomname);
        auto room = std::make_shared<Room>(roomname, id, db_);
        rooms_[roomname] = room;
        return room;
    }
}

bool Server::remove_room(const std::string& roomname){
    int id = db_.getRoomID(roomname);
    if(id != -1) { db_.deleteRoom(id); }
    
    return rooms_.erase(roomname) > 0;
}