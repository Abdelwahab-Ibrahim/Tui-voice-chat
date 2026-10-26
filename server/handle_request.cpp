#include <handle_request.hpp>
#include <unordered_map>
#include <room.hpp>
#include "user_repo.hpp"
#include <database.hpp>
#include <utils.hpp>
#include <cctype>
#include <exception>
#include <string>

using namespace std;

Database database("DB/users.db");
UserRepo userRepo(database);

struct Req
{
    string method;
    string body;
};

Req dispatch(const string &request)
{
    Req req;

    size_t sep = request.find(' ');

    if (sep == string::npos)
    {
        req.method = request;
        req.body = "";
        return req;
    }

    req.method = request.substr(0, sep);
    req.body = request.substr(sep + 1);

    return req;
}

pair<string, string> parseCredentials(const string &body)
{
    size_t sep = body.find('%');

    if (sep == string::npos)
    {
        return {"", ""};
    }

    string username = body.substr(0, sep);
    string password = body.substr(sep + 1);

    return {username, password};
}

// int is the room key
unordered_map<int, Room> Rooms;

// register	username%password     returns=====> ok or taken or err
// login	username%password     returns=====> ok or err
// create	username              returns=====> roomkey or err
// join	    username%room_key          returns=====> ok or err
// leave   	username%room_key          returns=====> ok or err
#include <iostream>
std::string handle_request(const std::string &request)
{

    Req req = dispatch(request);
    std::cout << request << std::endl
              << req.method << std::endl;

    if (req.method == "PING")
    {
        return "PONG";
    }
    else if (req.method == "REGISTER")
    {
        auto [username, password] = parseCredentials(req.body);
        RegisterUserResult result = userRepo.insert(username, password);
        if (result == RegisterUserResult::Created)
        {
            return "OK";
        }
        else if (result == RegisterUserResult::UsernameTaken)
        {
            return "TAKEN";
        }
    }
    else if (req.method == "LOGIN")
    {
        auto [username, password] = parseCredentials(req.body);
        bool isok = userRepo.verifyLogin(username, password);
        std::cout << isok << std::endl;
        if (isok)
        {
            return "OK";
        }
        // return "ERR";
    }
    else if (req.method == "CREATE")
    {
        auto user = userRepo.findByUsername(req.body);
        if (!user.has_value())
        {
            return "ERR";
        }

        int key;
        do
        {
            key = generateRandomInt();
        } while (Rooms.find(key) != Rooms.end());

        Rooms.emplace(key, Room(key, user->id));
        return to_string(key);
    }
    else if (req.method == "JOIN")
    {
        auto [username, skey] = parseCredentials(req.body);
        auto user = userRepo.findByUsername(username);
        if (!user.has_value())
        {
            return "ERR";
        }
        int key = stoi(skey);
        auto ro = Rooms.find(key);
        if(ro != Rooms.end()){
            ro->second.addMember(user->id);
            return "OK";
        }
    }
    else if (req.method == "LEAVE")
    {
        auto [username, skey] = parseCredentials(req.body);
        auto user = userRepo.findByUsername(username);
        if (!user.has_value())
        {
            return "ERR";
        }
        int key = stoi(skey);
        auto ro = Rooms.find(key);
        if (ro != Rooms.end())
        {
            ro->second.removeMember(user->id);
            return "OK";
        }
    }

    return "GetTheFuckOutOfHere";
}