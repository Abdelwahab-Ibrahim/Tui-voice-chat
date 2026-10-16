#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>

int main()
{
    try
    {
        SQLite::Database database("voice_chat.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        database.exec("CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY, "
                      "username TEXT NOT NULL UNIQUE, "
                      "password TEXT NOT NULL)");

        const std::string username = "demo";
        const std::string password = "demo-password";

        SQLite::Statement insertUser(
            database,
            "INSERT OR IGNORE INTO users (username, password) VALUES (?, ?)");
        insertUser.bind(1, username);
        insertUser.bind(2, password);
        insertUser.executeStep();

        SQLite::Statement findUser(
            database,
            "SELECT username, password FROM users WHERE username = ?");
        findUser.bind(1, username);

        if (!findUser.executeStep() || findUser.getColumn(0).getString() != username || findUser.getColumn(1).getString() != password)
        {
            std::cerr << "SQLite user query test failed\n";
            return EXIT_FAILURE;
        }

        std::cout << "SQLite user query test passed for username: " << username << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << "SQLite error: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
