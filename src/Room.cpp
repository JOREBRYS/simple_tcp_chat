#include "Room.h"
#include "Session.h"
#include "Database.h"
#include <chrono>

using namespace std;

void Room::add_participant(const std::shared_ptr<Session> session){
    participants_.push_back(session);
}

void Room::remove_participant(const std::shared_ptr<Session> session){
    for(auto it = participants_.begin(); it != participants_.end(); it++) {
        if(session == it->lock()) { participants_.erase(it); break; }
    }
}

void Room::broadcast(const std::string& text, const std::shared_ptr<Session> sender){
    Message mes;
    mes.userID = sender->getUserID();
    mes.username = sender->getUserName();
    mes.text = text;
    mes.timestamp = chrono::system_clock::now();

    add_message(mes);

    for(auto& i : participants_){
        std::shared_ptr<Session> ses = i.lock();
        if(i.lock() != sender) { ses->receive_message(mes); }
        else {ses->receive_message(mes, "YOU"); }
    }
}

void Room::add_message(const Message& message){
    history_.push_back(message);
    db_.saveMessage(message, info_.id_);
}

vector<Message> Room::get_history(int N){
    if( N < 1) {return{}; }

    return db_.getHistory(info_.id_, N);
}