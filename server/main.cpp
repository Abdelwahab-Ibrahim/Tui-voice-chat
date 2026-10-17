#include <iostream>
#include <exception>

#include "database.hpp"
#include "user_repo.hpp"

using namespace std;

int main()
{
    try
    {
        Database database("voicechat.db");
        database.initialize();

        UserRepo users(database);
while(true){


        string username;
        string password;

        cout << "=== Register ===\n";
        cout << "Username: ";
        cin >> username;

        cout << "Password: ";
        cin >> password;

        RegisterUserResult result = users.insert(username, password);

        if (result == RegisterUserResult::Created)
        {
            cout << "Registration successful!\n";
        }
        else
        {
            cout << "Username already exists.\n";
        }

        cout << "\n=== Login ===\n";

        cout << "Username: ";
        cin >> username;

        cout << "Password: ";
        cin >> password;

        if (users.verifyLogin(username, password))
        {
            cout << "Login successful!\n";
        }
        else
        {
            cout << "Invalid username or password.\n";
        }
    }
}
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}