#pragma once

#include <string>
#include <cstdint>
#include <memory>

class ServerSocket {
public:
    ServerSocket(int port);
    ~ServerSocket();
    
    // Start listening for connections
    bool listen();
    
    // Accept incoming client connection
    int accept();
    
    // Close server socket
    void close();
    
    // Get server socket file descriptor
    int getSocketFd() const { return socketFd; }
    
    // Check if listening
    bool isListening() const { return listening; }

private:
    int socketFd;
    int port;
    bool listening;
    
    // Helper: Create socket
    bool createSocket();
};
