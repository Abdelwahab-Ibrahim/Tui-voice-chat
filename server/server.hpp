
#ifndef SERVER_HPP
#define SERVER_HPP

class Server
{
private:
    int server_fd;
    int port;

    void handleClient(int client_fd);

public:
    explicit Server(int port);
    ~Server();

    bool start();
    void run();
};

#endif
