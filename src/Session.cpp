#include "Session.h"
#include "Server.h"
#include "Room.h"

    void Session::join_room(const std::string& roomName){
        if(current_room_ && current_room_->getName() == roomName) {
            do_write("You already have join that room.\n");
            return;
        }

        auto it = find_if(
            joined_rooms_.begin(),
            joined_rooms_.end(),
            [roomName] (const RoomConf& r) {return r.name_ == roomName; }
        );

        current_room_ = server_.get_or_create_room(roomName);
        current_room_->add_participant(shared_from_this());

        if(it == joined_rooms_.end()) {
            joined_rooms_.emplace_back(
                current_room_->getID(),
                current_room_->getName()
            );
        }

        do_write("You joined successfully\n");
    }

    void Session::return_to_menu(){
        if(!current_room_){
            do_write("You are not in a room.\n");
            return;           
        }

        int id = current_room_->getID();
        current_room_->remove_participant(shared_from_this());
        current_room_.reset();
    }

    void Session::leave(){
        if(!current_room_){
            do_write("You are not in a room.\n");
            return;
        }

        int id = current_room_->getID();
        auto it = find_if(
            joined_rooms_.begin(),
            joined_rooms_.end(),
            [id] (const RoomConf& r) {return r.id_ == id; }
        );
        if(it != joined_rooms_.end()) {joined_rooms_.erase(it); }

        db_.leaveRoom(userID_, current_room_->getID());
        current_room_->remove_participant(shared_from_this());
        current_room_.reset();
        do_write("You left successfully\n");
    }

    void Session::send_message(const std::string& text){
        if(!current_room_){
            do_write("You are not in a room.\n");
            return;
        }

        current_room_->broadcast(text, shared_from_this());
    }

    void Session::send_history(){
        if(!current_room_){
            do_write("You are not in a room.\n");
            return;
        }

        auto messages = current_room_->get_history(20);

        for(auto it : messages){
            do_write(it.username + " at " + ConvertTimeToString(it.timestamp) + " : " + it.text + "\n");
        }
    }