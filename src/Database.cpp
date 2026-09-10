#include "Database.h"

#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <chrono>

using namespace std;

Database::Database(){
    int openResult = sqlite3_open("chat.db", &db_);
    if(openResult != SQLITE_OK){
        std::string message = sqlite3_errmsg(db_);
        throw std::runtime_error(message.c_str());
    }

    if(!createTables()){
        throw std::runtime_error("error create table");
    }
}

Database::~Database(){
    if(!db_) {sqlite3_close(db_); }
}

string Database::hashPassword(const string& password){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha;
    SHA256_Init(&sha);

    SHA256_Update(&sha, password.c_str(), password.size());
    SHA256_Final(hash, &sha);
    
    std::stringstream ss;

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

int Database::registerUser(const string& userName,
    const string& password){
    if(userName.empty() || password.empty()){ return -1; }

    string passwordHash = hashPassword(password);

    sqlite3_stmt* statement;
    int prepareResult = sqlite3_prepare_v2(
        db_,
        "insert into users(username, password) values (?, ?);",
        -1,
        &statement,
        nullptr
    );

    if(prepareResult != SQLITE_OK){
        return -1;
    }

    sqlite3_bind_text(
        statement,
        1,
        userName.c_str(),
        -1,
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        statement,
        2,
        passwordHash.c_str(),
        -1,
        SQLITE_STATIC
    );

    int stepResult = sqlite3_step(statement);
    if(stepResult != SQLITE_DONE){
        if(stepResult == SQLITE_CONSTRAINT){
            std::cerr << "User already exists." << '\n';
        }
        sqlite3_finalize(statement);
        return -1;
    }

    sqlite3_finalize(statement);
    return sqlite3_last_insert_rowid(db_);
}


int Database::loginUser(const string& userName,
    const string& password){
    sqlite3_stmt* statement;
    int prepareResult = sqlite3_prepare_v2(
        db_,
        "select id, password from users where username = ?;",
        -1,
        &statement,
        nullptr
    );
    if(prepareResult != SQLITE_OK) {
        sqlite3_finalize(statement);
        return -1;
    }

    sqlite3_bind_text(
        statement,
        1,
        userName.c_str(),
        -1,
        nullptr
    );

    int stepResult = sqlite3_step(statement);
    if(stepResult != SQLITE_ROW) {
        sqlite3_finalize(statement);
        return -1;
    }

    int id = sqlite3_column_int(statement, 0);
    const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));

    std::string passwordHash = hashPassword(password);

    if(passwordHash != storedHash){
        sqlite3_finalize(statement);
        return -1;
    }

    sqlite3_finalize(statement);
    return id;
}

bool Database::createTables(){
    int foreignkeyOn = sqlite3_exec(
        db_,
        "PRAGMA foreign_key = ON;",
        nullptr,
        nullptr,
        nullptr 
    );
    if(foreignkeyOn != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return false;
    }

    string sqlQuery_createUsers = "create table if not exists users("
        "id integer primary key autoincrement,"
        "username text unique not null,"
        "password text not null,"
        "created_at datetime default current_timestamp);";

    string sqlQuery_createRooms = "create table if not exists rooms("
            "id integer primary key autoincrement,"
            "name text unique not null,"
            "created_at datetime default current_timestamp);";

    string sqlQuery_createMessages = "create table if not exists messages("
        "id integer primary key autoincrement,"
        "room_id integer,"
        "user_id integer not null,"
        "message_text text not null,"
        "timestamp integer not null,"
        "foreign key (user_id) references users (id) on delete restrict,"
        "foreign key (room_id) references rooms (id) on delete cascade);";

    string sqlQuery_createUserToRooms = "create table if not exists user_to_rooms("
        "user_id integer not null,"
        "room_id integer not null,"
        "primary key(user_id, room_id),"
        "foreign key (user_id) references users (id) on delete cascade,"
        "foreign key (room_id) references rooms (id) on delete cascade);";

    int createUsersResult = sqlite3_exec(db_, sqlQuery_createUsers.c_str(), nullptr, nullptr, nullptr);
    if(createUsersResult != SQLITE_OK){
        string message = sqlite3_errmsg(db_);
        cerr << message << '\n';
        return false;
    }

    int createRoomsResult = sqlite3_exec(db_, sqlQuery_createRooms.c_str(), nullptr, nullptr, nullptr);
    if(createRoomsResult != SQLITE_OK){
        string message = sqlite3_errmsg(db_);
        cerr << message << '\n';
        return false;
    }

    int createMessagesResult = sqlite3_exec(db_, sqlQuery_createMessages.c_str(), nullptr, nullptr, nullptr);
        if(createMessagesResult != SQLITE_OK){
        string message = sqlite3_errmsg(db_);
        cerr << message << '\n';
        return false;
    }

    int createUserToRooms = sqlite3_exec(db_, sqlQuery_createUserToRooms.c_str(), nullptr, nullptr, nullptr);
    if(createUserToRooms != SQLITE_OK){
        string message = sqlite3_errmsg(db_);
        cerr << message << '\n';
        return false;
    }

    return true;
} 

int Database::getRoomID(const string& roomname){
    std::string query = "select id from rooms where name = ?;";
    sqlite3_stmt* stmt;

    int res = sqlite3_prepare_v2(
        db_,
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );
    if(res != SQLITE_OK){
        std::cerr << sqlite3_errmsg(db_);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, roomname.c_str(), -1, SQLITE_STATIC);

    int exeRes = sqlite3_step(stmt);

    int id = -1;
    if(exeRes == SQLITE_ROW){
        id = sqlite3_column_int(stmt, 0);
    }
    else if(exeRes != SQLITE_DONE ){
        cerr <<sqlite3_errmsg(db_);
    }
    
    sqlite3_finalize(stmt);
    return id;
}

int Database::createRoom(const string& roomname){
    string query = "insert into rooms(name, created_at) values (?, ?);";
    sqlite3_stmt* stmt;

    int res = sqlite3_prepare_v2(
        db_, 
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );
    if(res != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return -1;
    }

    sqlite3_bind_text(
        stmt,
        1,
        roomname.c_str(),
        -1,
        nullptr
    );

    sqlite3_bind_int(
        stmt,
        2,
        time(0)
    );

    int stepRes = sqlite3_step(stmt);
    if(stepRes != SQLITE_DONE){
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return sqlite3_last_insert_rowid(db_);
}

bool Database::deleteRoom(const int roomID){
    string query = "delete from rooms where id = ?";
    sqlite3_stmt *stmt;

    int res = sqlite3_prepare_v2(
        db_,
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );
    if(res != SQLITE_OK){
        cerr << sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_int(
        stmt,
        1,
        roomID
    );

    int stepres = sqlite3_step(stmt);
    if(stepres != SQLITE_DONE){
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::saveMessage(const Message& message, int roomID){
    string query = "insert into messages(user_id, room_id, message_text, timestamp) values"
        "(?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    int res = sqlite3_prepare_v2(
        db_,
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );
    if(res != SQLITE_OK){
        cerr << sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_int(
        stmt,
        1,
        message.userID
    );
    sqlite3_bind_int(
        stmt,
        2,
        roomID
    );
    sqlite3_bind_text(
        stmt,
        3,
        message.text.c_str(),
        -1,
        nullptr
    );
        sqlite3_bind_int(
        stmt,
        4,
        time(0)
    );

    int stepres = sqlite3_step(stmt);
    if(stepres != SQLITE_DONE){
        cerr << sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

vector<Message> Database::getHistory(int roomID, int N){
    if(N < 1) { return{}; }

    string query = "select m.user_id, u.username, m.message_text, m.timestamp "
        "from users u "
        "join messages m on u.id = m.user_id "
        "where room_id = ? "
        "order by m.timestamp "
        "limit ?";
    sqlite3_stmt* stmt;
    
    int res = sqlite3_prepare_v2(
        db_,
        query.c_str(),
        -1,
        &stmt,
        nullptr
    );
    if(res != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return {};
    }
    sqlite3_bind_int(
        stmt,
        1,
        roomID
    );
    sqlite3_bind_int(
        stmt,
        2,
        N
    );

    vector<Message> messages{};
    while(sqlite3_step(stmt) == SQLITE_ROW){
       messages.emplace_back(
        sqlite3_column_int(stmt, 0),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
        chrono::system_clock::from_time_t(sqlite3_column_int(stmt, 3))
       );
    }

    sqlite3_finalize(stmt);
    return messages;
}

vector<RoomConf> Database::getRoomsList(int userID){
    auto query = "select r.id, r.name "
        "from users u "
        "join user_to_rooms ur on u.id = ur.user_id "
        "join rooms r on ur.room_id = r.id "
        "where u.id = ?;";
    sqlite3_stmt* stmt;

    int prepared = sqlite3_prepare_v2(
        db_,
        query,
        -1,
        &stmt,
        nullptr
    );
    if(prepared != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return {};
    }

    sqlite3_bind_int(
        stmt,
        1,
        userID
    );

    vector<RoomConf> confs{};
    while(sqlite3_step(stmt) == SQLITE_ROW){
        confs.emplace_back(
            sqlite3_column_int(stmt, 0),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
        );
    }

    sqlite3_finalize(stmt);
    return confs;
}

void Database::leaveRoom(int userID, int roomID){
    auto query = "delete from user_to_rooms where (user_id = ?) and (room_id = ?)";
    sqlite3_stmt *stmt;

    int prepared = sqlite3_prepare_v2(
        db_,
        query,
        -1,
        &stmt,
        nullptr
    );
    if(prepared != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return;
    }

    sqlite3_bind_int(
        stmt,
        1,
        userID
    );
    sqlite3_bind_int(
        stmt,
        2,
        roomID
    );

    int steped = sqlite3_step(stmt);
    if(steped != SQLITE_DONE){
        cerr << sqlite3_errmsg(db_);
    }
    sqlite3_finalize(stmt);
    return;
}

void Database::joinRoom(int userID, int roomID){
    auto query = "insert into user_to_rooms values"
        "(?, ?);";
    sqlite3_stmt* stmt;

    int prepared = sqlite3_prepare_v2(
        db_,
        query,
        -1,
        &stmt,
        nullptr
    );
    if(prepared != SQLITE_OK){
        cerr << sqlite3_errmsg(db_) << '\n';
        return;
    }

    sqlite3_bind_int(
        stmt,
        1,
        userID
    );
    sqlite3_bind_int(
        stmt,
        2,
        roomID
    );

    int steped = sqlite3_step(stmt);
    if(steped != SQLITE_DONE){
        cerr << sqlite3_errmsg(db_) << '\n';
    }
}