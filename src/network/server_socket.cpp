#include "server_socket.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INVALID_SOCKET -1

// Constructor
ServerSocket::ServerSocket(int port)
    : socketFd(INVALID_SOCKET), port(port), listening(false) {
}

// Destructor
ServerSocket::~ServerSocket() {
    close();
}

// Create socket
bool ServerSocket::createSocket() {
    socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket\n";
        return false;
    }
    
    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket option\n";
        ::close(socketFd);
        socketFd = INVALID_SOCKET;
        return false;
    }
    
    return true;
}

// Listen for connections
bool ServerSocket::listen() {
    if (!createSocket()) {
        return false;
    }
    
    // Bind socket to port
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    
    if (bind(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket to port " << port << "\n";
        ::close(socketFd);
        socketFd = INVALID_SOCKET;
        return false;
    }
    
    // Listen for incoming connections
    if (::listen(socketFd, 5) < 0) {
        std::cerr << "Failed to listen on socket\n";
        ::close(socketFd);
        socketFd = INVALID_SOCKET;
        return false;
    }
    
    listening = true;
    std::cout << "Server listening on port " << port << "\n";
    return true;
}

// Accept client connection
int ServerSocket::accept() {
    if (!listening || socketFd == INVALID_SOCKET) {
        std::cerr << "Server not listening\n";
        return INVALID_SOCKET;
    }
    
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    int clientSocket = ::accept(socketFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection\n";
        return INVALID_SOCKET;
    }
    
    // Print client info
    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);
    
    std::cout << "New client connected: " << clientIp << ":" << clientPort << " (fd=" << clientSocket << ")\n";
    
    return clientSocket;
}

// Close server socket
void ServerSocket::close() {
    if (socketFd != INVALID_SOCKET) {
        ::close(socketFd);
        socketFd = INVALID_SOCKET;
    }
    listening = false;
}
