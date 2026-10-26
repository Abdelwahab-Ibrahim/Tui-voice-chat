#include <iostream>
#include <exception>

#include "database.hpp"

#include "utils.hpp"
#include "server.hpp"
#include <unordered_map>

// includes-


using namespace std;

int main()
{

   
    // =======================

    
    Server server(8080);

    if (!server.start())
    {
        return 1;
    }

    server.run();

    return 0;
}