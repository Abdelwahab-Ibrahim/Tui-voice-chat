
#include "database.hpp"
#include <iostream>
#include <stdexcept>

Database::Database(const std::string &filename)
{
    int result = sqlite3_open(filename.c_str(), &db);
    Database::initialize();
    if (result != SQLITE_OK)
    {
        std::cerr << "failed to create database" << std::endl;
    }
    else
    {
        std::cout << "database createdd successfully" << std::endl;
        sqlite3_extended_result_codes(db, 1);
    }
}

Database::~Database()
{
    sqlite3_close(db);
}

void Database::initialize()
{
    // TODO:
    // Ensure the connection is open.
    // Enable foreign keys.
    // Create the required tables.

    char *err;
    char *sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT,username TEXT NOT NULL UNIQUE,password TEXT NOT NULL)";
    int result = sqlite3_exec(db, sql, NULL, NULL, &err);

    if (result != SQLITE_OK)
    {
        std::cerr << "ERROR: " << err << std::endl;
    }
    else
    {
        std::cout << "database createdd successfully" << std::endl;
    }
}

void Database::createTables()
{
    // TODO:
    // Create users:
    //   id INTEGER PRIMARY KEY
    //   username TEXT NOT NULL UNIQUE
    //   password TEXT NOT NULL
    //
    // Create rooms:
    //   id INTEGER PRIMARY KEY
    //   name TEXT NOT NULL UNIQUE
    //   owner_user_id INTEGER REFERENCES users(id)
    //
    // Use CREATE TABLE IF NOT EXISTS.
}

void Database::execute(const std::string &sql)
{
    // TODO:
    // Execute SQL using sqlite3_exec().
    // Capture and report SQLite errors.
    // Release SQLite-allocated error messages.
    // Propagate errors to the caller.
}