#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

class TcpServer {
    public:
        typedef std::function<std::string(const std::string&)> MessageHandler;

        TcpServer(unsigned short port,
                  std::size_t maxClients,
                  MessageHandler handler);
        ~TcpServer();

        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;

        bool start(std::string& error);
        void update();
        void stop();
    private:
    struct Client {
            int fd;
            std::string input;
        };

        int _listenFd;
        unsigned short _port;
        std::size_t _maxClients;
        MessageHandler _handler;
        std::vector<Client> _clients;

        void acceptClient();
        void handleClient(int fd);
        void removeClient(int fd);
        bool sendAll(int fd, const std::string& message);
};