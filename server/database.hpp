#pragma once

#include <sqlite3.h>
#include <string>

class UserRepo;
class RoomRepo;

class Database
{
public:
    explicit Database(const std::string &filename);
    ~Database();

    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    void initialize();

private:
    sqlite3 *db = nullptr;

    void execute(const std::string &sql);
    void createTables();

    friend class UserRepo;
    friend class RoomRepo;
};