#include <iostream>
#include <exception>

#include "database.hpp"
#include "user_repo.hpp"
#include "room.hpp"
#include "utils.hpp"
#include "server.hpp"
#include <unordered_map>

// includes-
// #include <sys/types.h>
// #include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

using namespace std;

int main()
{

   
    // =======================

    unordered_map<int, Room> Rooms;

    Server server(8080);

    if (!server.start())
    {
        return 1;
    }

    server.run();

    return 0;
}