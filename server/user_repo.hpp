#pragma once

#include "database.hpp"

#include <optional>
#include <string>

struct User
{
    int id;
    std::string username;
    std::string password;
};

enum class RegisterUserResult
{
    Created,
    UsernameTaken,
    Error
};

class UserRepo
{
public:
    explicit UserRepo(Database &database);

    RegisterUserResult insert(
        const std::string &username,
        const std::string &password);

    std::optional<User> findByUsername(
        const std::string &username);

    bool verifyLogin(
        const std::string &username,
        const std::string &password);

private:
    Database &database;
};