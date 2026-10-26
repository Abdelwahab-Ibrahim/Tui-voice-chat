#include "user_repo.hpp"
#include <string>
#include <stdexcept>
#include <exception>
#include<iostream>

UserRepo::UserRepo(Database &database)
    : database(database)
{
}

RegisterUserResult UserRepo::insert(
    const std::string &username,
    const std::string &password)
{
    if (username.empty() || password.empty())
    {
        return RegisterUserResult::Error;
    }

    sqlite3_stmt *stmt = nullptr;

    const char *sql =
        "INSERT INTO users (username, password) VALUES (?, ?);";

    int result = sqlite3_prepare_v2(
        database.db, sql, -1, &stmt, nullptr);

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(database.db));
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);

    if (result != SQLITE_DONE)
    {
        int errorCode = sqlite3_extended_errcode(database.db);

        if (errorCode == SQLITE_CONSTRAINT_UNIQUE)
        {
            sqlite3_finalize(stmt);
            return RegisterUserResult::UsernameTaken;
        }
        else
        {
            std::string error = sqlite3_errmsg(database.db);

            sqlite3_finalize(stmt);
            return RegisterUserResult::Error;
        }
    }

    sqlite3_finalize(stmt);
    return RegisterUserResult::Created;
}

std::optional<User> UserRepo::findByUsername(
    const std::string &username)
{
    if (username.empty())
    {
        return std::nullopt;
    }

    sqlite3_stmt *stmt = nullptr;

    const char *sql =
        "SELECT id, username, password "
        "FROM users WHERE username = ?;";

    int result = sqlite3_prepare_v2(
        database.db, sql, -1, &stmt, nullptr);

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(database.db));
    }

    result = sqlite3_bind_text(
        stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (result != SQLITE_OK)
    {
        std::string error = sqlite3_errmsg(database.db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    result = sqlite3_step(stmt);

    if (result == SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return std::nullopt; // No matching user
    }

    if (result != SQLITE_ROW)
    {
        std::string error = sqlite3_errmsg(database.db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(error);
    }

    User user;

    user.id = sqlite3_column_int(stmt, 0);

    const auto *u_name = sqlite3_column_text(stmt, 1);
    const auto *ps_word = sqlite3_column_text(stmt, 2);

    user.username = u_name
                        ? reinterpret_cast<const char *>(u_name)
                        : "";

    user.password = ps_word
                        ? reinterpret_cast<const char *>(ps_word)
                        : "";

    sqlite3_finalize(stmt);
    return user;
}

bool UserRepo::verifyLogin(
    const std::string &username,
    const std::string &password)
{
    if (username.empty() || password.empty())
    {
        return false;
    }

    std::optional<User> user = findByUsername(username);

    if (!user.has_value())
    {
        return false;
    }

    return user->password == password;
}