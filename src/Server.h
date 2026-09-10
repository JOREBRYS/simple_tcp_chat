#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <map>
#include "Session.h"
#include "Room.h"
#include <memory>

class Server{
    private:
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    Database& db_;
    std::map<std::string, std::shared_ptr<Room>> rooms_; 

    void do_accept(){
        acceptor_.async_accept(
            [this](const boost::system::error_code& ec,
                boost::asio::ip::tcp::socket socket){
                    if(!ec){
                        std::cout << "Client connected!\n";
                        std::make_shared<Session>(io_context_, std::move(socket), db_, *this)->start();
                    }
                    else{
                        std::cerr << "Ошибка accept: " << ec.message() << '\n'; 
                    }
                    do_accept();
                }
        );
    }

    public:
    Server(boost::asio::io_context& io,
        short port, Database& db): io_context_(io),
        acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        db_(db){
            do_accept();
    }

    std::shared_ptr<Room> get_or_create_room(const std::string& roomname);

    bool remove_room(const std::string& roomname);
};

#endif