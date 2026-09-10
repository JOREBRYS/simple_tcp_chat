#include "Server.h"
#include "Database.h"
#include <boost/asio.hpp>

int main(){
    try{
        boost::asio::io_context io;
        Database db;
        Server serv(io, 8080, db);

        std::cout << "Server is open and listening the port 8080.\n";

        io.run();
    }
    catch(const std::exception& e){
        std::cerr << "Err: " << e.what() << '\n'; 
    }
    return 0;
}