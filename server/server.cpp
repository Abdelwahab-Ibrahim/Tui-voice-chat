
#include "server.hpp"
#include<handle_request.hpp>

#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>



Server::Server(int port)
    : server_fd(-1), port(port) {}

Server::~Server()
{
    if (server_fd != -1)
    {
        close(server_fd);
    }
}

bool Server::start()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket");
        return false;
    }

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        return false;
    }

    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("bind");
        return false;
    }

    if (listen(server_fd, 5) == -1)
    {
        perror("listen");
        return false;
    }

    std::cout << "Server listening on port "
              << port << '\n';

    return true;
}

void Server::run()
{
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(
            server_fd,
            (sockaddr *)&client_addr,
            &client_len);

        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        std::cout << "Client connected\n";

        handleClient(client_fd);

        close(client_fd);

        std::cout << "Client disconnected\n";
    }
}

void Server::handleClient(int client_fd)
{
    char buffer[4096];

    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0)
    {
        if (bytes_received == -1)
        {
            perror("recv");
        }
        return;
    }

    std::string request(buffer, bytes_received);
    std::string response = handle_request(request);

    size_t total_sent = 0;

    while (total_sent < response.size())
    {
        ssize_t bytes_sent = send(
            client_fd,
            response.data() + total_sent,
            response.size() - total_sent,
            0);

        if (bytes_sent <= 0)
        {
            if (bytes_sent == -1)
            {
                perror("send");
            }
            return;
        }

        total_sent += bytes_sent;
    }
}
