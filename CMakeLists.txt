cmake_minimum_required(VERSION 3.10)
project(tcp_chat VERSION 1.0)

find_package(Boost REQUIRED COMPONENTS system)

set(SQLITE3_INCLUDE_DIR "/usr/include")
set(SQLITE3_LIBRARY "/usr/lib/x86_64-linux-gnu/libsqlite3.so")
find_package(SQLite3 REQUIRED)

find_package(OpenSSL REQUIRED)

add_executable(tcp_chat src/Main.cpp src/Database.cpp src/Session.cpp src/Server.cpp src/Room.cpp)
target_link_libraries(tcp_chat ${Boost_LIBRARIES} sqlite3 ssl crypto)
target_include_directories(tcp_chat PRIVATE ${OPENSSL_INCLUDE_DIR} ${SQLITE3_INCLUDE_DIR})
target_compile_features(tcp_chat PRIVATE cxx_std_17)