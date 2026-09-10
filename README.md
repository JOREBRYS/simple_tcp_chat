# simple_tcp_chat
The simple tcp chat written with Boost asio.

In subdirectory client there are soure of the client.
In this project I use following technologies:
chrono - for working with time,
Boost asio - for networking,
SQLite - for storaging users data, messages,
OpenSSL - for hashing user passwords,
CMake - for linking, generating and building.

The chat supports only rooms.
list of available commands:

  PONG -> you will got PONG
  
  HELP -> you just have seen what it does
  
  REGISTER <login> <password> -> you register
  
  LOGIN <login> <password> -> you log in
  
  JOIN <roomname> -> you join or create a room
  LEAVE -> you leave a room
  
  MSG <message> -> you send your message
  
  HISTORY -> you get room's history
