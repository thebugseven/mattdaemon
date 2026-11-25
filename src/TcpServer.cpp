#include "TcpServer.hpp"

TcpServer::TcpServer(unsigned short port, std::size_t maxClients, MessageHandler handler) : _port(port), _maxClients(maxClients), _handler(handler) {
    _listenFd = -1;
}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start(std::string& error) {
    if (_listenFd >= 0)
        return true;

    if (_maxClients == 0) {
        error = "Server must allow at least one client";
        return false;
    }

    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd == -1) {
        error = std::string("socket: ") + std::strerror(errno);
        return false;
    }

    int reuseAddress = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddress, sizeof(reuseAddress)) == -1) {
        error = std::string("setsockopt: ") + std::strerror(errno);
        stop();
        return false;
    }

    int descriptorFlags = fcntl(_listenFd, F_GETFD);
    if (descriptorFlags == -1 || fcntl(_listenFd, F_SETFD, descriptorFlags | FD_CLOEXEC) == -1) {
        error = std::string("fcntl: ") + std::strerror(errno);
        stop();
        return false;
    }

    int statusFlags = fcntl(_listenFd, F_GETFL);
    if (statusFlags == -1 || fcntl(_listenFd, F_SETFL, statusFlags | O_NONBLOCK) == -1) {
        error = std::string("fcntl: ") + std::strerror(errno);
        stop();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(_port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(_listenFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        error = std::string("bind: ") + std::strerror(errno);
        stop();
        return false;
    }

    if (listen(_listenFd, static_cast<int>(_maxClients)) == -1) {
        error = std::string("listen: ") + std::strerror(errno);
        stop();
        return false;
    }

    return true;
}

void TcpServer::update() {
    if(_listenFd < 0)
        return;

    std::vector<pollfd> descriptors;
    descriptors.push_back({_listenFd, POLLIN, 0});

    for (const Client& client : _clients) 
        descriptors.push_back({client.fd, POLLIN, 0});

    int ready = poll(descriptors.data(), descriptors.size(), 0);
    if (ready <= 0) {
        if (ready == -1 && errno != EINTR)
            perror("poll");
        return;
    }

    if (descriptors[0].revents & POLLIN) 
        acceptClient();

    for (std::size_t i = 1; i < descriptors.size(); ++i) {
        int fd = descriptors[i].fd;
        short events = descriptors[i].revents;

        if (events == 0)
            continue;

        if (events & POLLIN)
            handleClient(fd);

        if (events & (POLLERR | POLLHUP | POLLNVAL))
            removeClient(fd);
    }
}

void TcpServer::stop() {
    for (Client& client : _clients)
        close(client.fd);

    _clients.clear();

    if (_listenFd >= 0) {
        close(_listenFd);
        _listenFd = -1;
    }
}

void TcpServer::acceptClient() {
    sockaddr_in clientAddress{};
    socklen_t addressLength = sizeof(clientAddress);

    int clientFd = accept(
        _listenFd,
        reinterpret_cast<sockaddr*>(&clientAddress),
        &addressLength
    );

    if (clientFd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
            perror("accept");
        }
        return;
    }

    if (_clients.size() >= _maxClients) {
        std::string response = "ERROR: server is full\n";
        response.push_back('\0');
        sendAll(clientFd, response);
        close(clientFd);
        return;
    }

    int flags = fcntl(clientFd, F_GETFD);
    if (flags == -1 || fcntl(clientFd, F_SETFD, flags | FD_CLOEXEC) == -1) {
            close(clientFd);
            return;
        }

    _clients.push_back({clientFd, ""});
}

void TcpServer::handleClient(int fd) {
    char buffer[4096];

    ssize_t bytesRead = recv(fd, buffer,sizeof(buffer), 0);

    if (bytesRead == 0) {
        removeClient(fd);
        return;
    }

    if (bytesRead < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            removeClient(fd);
        return;
    }

    for (Client& client : _clients) {
        if (client.fd != fd)
            continue;

        client.input.append(buffer, static_cast<std::size_t>(bytesRead));

        while (true) {
            std::size_t newline = client.input.find('\n');
            if (newline == std::string::npos)
                break;

            std::string message = client.input.substr(0, newline);
            client.input.erase(0, newline + 1);

            if (!message.empty() && message.back() == '\r')
                message.pop_back();

            std::string response = _handler(message);

            if (!response.empty() && response.back() != '\n')
                response += '\n';

            response.push_back('\0');

            if (!sendAll(fd, response)) {
                removeClient(fd);
                return;
            }
        }

        if (client.input.size() > 8192) 
            removeClient(fd);

        return;
    }
}

bool TcpServer::sendAll(int fd, const std::string& message) {
    std::size_t sent = 0;

    while (sent < message.size()) {
        ssize_t result = send(
            fd,
            message.data() + sent,
            message.size() - sent,
            MSG_NOSIGNAL
        );

        if (result < 0) {
            if (errno == EINTR)
                continue;
            return false;
        }

        sent += static_cast<std::size_t>(result);
    }

    return true;
}

void TcpServer::removeClient(int fd) {
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->fd == fd) {
            close(it->fd);
            _clients.erase(it);
            return;
        }
    }
}