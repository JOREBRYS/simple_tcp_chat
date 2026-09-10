#ifndef SESSION_H
#define SESSION_H

#include <vector>
#include <boost/asio.hpp>
#include <memory>
#include <iostream>
#include <cstddef>
#include <string>

#include "Database.h"
#include "Commands.h"
#include "Message.h"
#include "TimeToStringConverter.h"

class Server;
class Room;

class Session : public std::enable_shared_from_this<Session> {
    private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context& io_;
    std::vector<char> buffer_;
    std::string read_buffer_;
    Database& db_;
    std::shared_ptr<Room> current_room_;
    std::vector<RoomConf> joined_rooms_;
    Server& server_;
    int userID_;

    bool isLoggedIn_ = false;
    std::string username_;

    void do_read(){
        auto self = shared_from_this();

        socket_.async_read_some(
            boost::asio::buffer(buffer_),
            [this, self] (const boost::system::error_code& ec, std::size_t bytes_transferred){
                if(ec){
                    if(ec == boost::asio::error::eof) {
                        std::cerr << "Client disconnented" << '\n';
                    }
                    else{
                        std::cerr << "Unknown Error" << '\n';
                    }
                    return;
                }
                handle_read(ec, bytes_transferred);
            }
        );
    }

    void handle_read(const boost::system::error_code& ec,
        const std::size_t bytes_transferred){
        if(ec){
            return;
        }

        read_buffer_.append(buffer_.begin(), buffer_.begin() + bytes_transferred);

        std::size_t pos;
        while((pos = read_buffer_.find("\n")) != std::string::npos){
            std::string command = read_buffer_.substr(0, pos);
            read_buffer_.erase(0, pos + 1);
            handle_command(command);
        }

        do_read();
    }

    void handle_command(const std::string& command){
        std::cout << "receive command: " << command << '\n';

        std::string header = "";
        std::stringstream ss(command);
        ss >> header;

        if(!header.empty() && header.back() == '\r') {header.pop_back(); }
        if(header[0] == '/') { header.erase(header.begin()); }
        int commandId = getCommandID(header);

        std::vector<std::string> args{};
        std::string arg;
        int posstart = command.find('\"');
        if(posstart != std::string::npos){
            int endpos = command.find('\"', posstart + 1);
            if(endpos == std::string::npos) {
                do_write("wrong argument.\n");
                return;
            }

            args.emplace_back(
                command.substr(posstart + 1, endpos - posstart - 1)
            );
        }
        else{
            while(ss >> arg){
                if(arg[arg.size() - 1] == '\r') {arg.erase(arg.size() - 1); }
                args.push_back(arg);
            }
        }
 
        std::string responsible = "";
        switch(commandId){
            case static_cast<int>(Command::PING) : {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                responsible = "PONG\n";
                break;
            }
            case static_cast<int>(Command::HELP) :{
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                responsible = "All commands:\n"
                    "PONG -> you will got \"PONG\"\n"
                    "HELP -> you just have seen what it does\n"
                    "/REGISTER <login> <password> -> you register\n"
                    "/LOGIN <login> <password> -> you log in\n"
                    "/JOIN <roomname> -> you join or create a room\n"
                    "/LEAVE -> you leave a room\n"
                    "/MSG <message> -> you send your message\n"
                    "/HISTORY -> you get room\'s history\n";
                break; 
            }
            case static_cast<int>(Command::REGISTER) : {
                const int argsRequiredAmount = 2;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }
                
                handle_register(args[0], args[1]);

                break;
            }
            case static_cast<int>(Command::LOGIN) : {
                const int argsRequiredAmount = 2;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }

                handle_login(args[0], args[1]);

                break;
            }
            case static_cast<int>(Command::JOIN): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                const int argsRequiredAmount = 1;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }

                if(current_room_){
                    return_to_menu();
                }
                
                join_room(args[0]);
                db_.joinRoom(userID_, current_room_->getID());

                break;
            }
            case static_cast<int>(Command::LEAVE): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                const int argsRequiredAmount = 0;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }

                leave();

                break;
            }
            case static_cast<int>(Command::MSG): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                const int argsRequiredAmount = 1;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }

                send_message(args[0]);

                break;
            }
            case static_cast<int>(Command::HISTORY): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }

                const int argsRequiredAmount = 0;
                if(!CompareArgsAmount(args, argsRequiredAmount)){
                    do_write("There are no enough arguments\n");
                    return;
                }

                send_history();

                break;
            }
            case static_cast<int>(Command::LIST): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }
                do_write("List of joined rooms:\n");
                
                loadJoinedRooms();
                int size = joined_rooms_.size();
                for(int i = 0; i < size; i++){
                    do_write(joined_rooms_[i].name_ + "\n");
                }
                do_write("\n");
                break;
            }
            case static_cast<int>(Command::MENU): {
                if(!isLoggedIn_){
                    do_write("You must be logged in\n");
                    return;
                }
                if(!current_room_){
                    do_write("You are not in any room.\n");
                    return;
                }

                return_to_menu();
                do_write("You returned to the menu.\n");
                break;
            }
            default:{
                responsible = "Unknown command\n";
                break;
             }
        }

        if(!responsible.empty()) {do_write(responsible); }
    }

    void do_write(const std::string& responsible){
        auto self = shared_from_this();

        auto data = std::make_shared<std::string>(responsible);

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(*data),
            [this, self, data] (const boost::system::error_code& ec, std::size_t){
                if(ec){
                    if(ec == boost::asio::error::eof) {
                        std::cerr << "Client disconnented" << '\n';
                    }
                    else{
                        std::cerr << "Unknown Error" << '\n';
                    }
                    return;
                }
                do_read();
            }
        );
    }

    void handle_register(const std::string& login, const std::string& password){
        int id = db_.registerUser(login, password);

        if(id == -1){
            do_write("Something was wrong \n");
            return;
        }

        userID_ = id;
        username_ = login;
        do_write("SECCUSSFUL\n");

        handle_login(login, password);
    }

    void handle_login(const std::string& login, const std::string& password){
        int id = db_.loginUser(login, password);

        if(id == -1){
            do_write("Login or password is wrong\n");
            return;
        }
        username_ = login;
        userID_ = id;
        do_write("LOGIN SUCCESSFUL. WELLCOME " + username_ + "\n");
        isLoggedIn_ = true;
    }

    template<typename T>
    bool CompareArgsAmount(const std::vector<T>& args, const unsigned int requiredAmount){
        if(args.size() != requiredAmount) {return false; }
        return true;
    }

    void join_room(const std::string& roomName);
    void leave();
    void send_message(const std::string& text);
    void send_history();
    void return_to_menu();

    void loadJoinedRooms(){
        joined_rooms_ = db_.getRoomsList(userID_);
    }

    public:
    Session(boost::asio::io_context& io, 
        boost::asio::ip::tcp::socket&& socket,
        Database& db,
        Server& ser): io_(io),
        socket_(std::move(socket)), buffer_(1024, 0),
        db_(db), server_(ser), current_room_() {}

    void start(){
        do_read();
    }

    int getUserID(){
        return userID_;
    }

    std::string getUserName(){
        return username_;
    }

    void receive_message(const Message& message, std::string sender = ""){
        if(sender.empty()) {sender = message.username; }
        do_write(sender + " at " + ConvertTimeToString(message.timestamp) + " : " + message.text + "\n");
    }
};

#endif