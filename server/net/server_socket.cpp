#include "server_socket.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define INVALID_SOCKET -1

ServerSocket::ServerSocket(int port)
    : socketFd(INVALID_SOCKET), port(port), listening(false) {}
    
ServerSocket::~ServerSocket() {
    close();
}

bool ServerSocket::createSocket() {
    socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == INVALID_SOCKET) {
        perror("socket");
        return false;
    }

    int opt = 1;
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return true;
}

bool ServerSocket::listen() {
    if (!createSocket()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }

    if (::listen(socketFd, 128) < 0) {
        perror("listen");
        return false;
    }

    listening = true;
    std::cout << "[Server] Listening on port " << port << std::endl;
    return true;
}

int ServerSocket::acceptClient() {
    return ::accept(socketFd, nullptr, nullptr);
}

int ServerSocket::getFd() const {
    return socketFd;
}

void ServerSocket::close() {
    if (socketFd != INVALID_SOCKET) {
        ::close(socketFd);
        socketFd = INVALID_SOCKET;
    }
    listening = false;
}
