#include <dispature.hpp>

std::string dispatchRequest(const std::string &request)
{
    if (request == "PING")
    {
        return "PONG\n";
    }

    if (request == "HELLO")
    {
        return "Hello, client!\n";
    }

    return "ERROR: Unknown request\n";
}