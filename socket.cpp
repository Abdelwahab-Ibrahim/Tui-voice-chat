#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
// create socket 
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == -1)
    {
        std::cerr << "cant create socket";
        return -1;
    }

    std::cout << "socket created successfully" << std::endl;
// bind a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(8080);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (sockaddr *)&hint, sizeof(hint)) == -1)
    {
        std::cerr << "cant bind socket";
        return 1;
    }

    std::cout << "socket bound successfully" << std::endl;


// mark as listenning 
    if (listen(server, SOMAXCONN) == -1)
    {
        std::cerr << "Listen failed\n";
        close(server);
        return 2;
    }

    std::cout << "Server is listening\n";

// connect to clint
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(
            server,
            (sockaddr *)&client_addr,
            &client_len);

        if (client_fd == -1)
        {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::cout << "Client connected!\n";


        char buffer[1024];
        // while (true)
        // {
            ssize_t bytes_received = recv(
                client_fd,
                buffer,
                sizeof(buffer),
                0);

            if (bytes_received <= 0)
            {
                break;
            }

            std::cout.write(buffer, bytes_received);
            std::cout << '\n';

            const char response[] = "Message received!\n";

            ssize_t bytes_sent = send(
                client_fd,
                response,
                sizeof(response) - 1,
                0);

            if (bytes_sent == -1)
            {
                perror("send");
            }

        // }
        close(client_fd);
        std::cout << "Client disconnected\n";
    }

// recieving 

    close(server);
}