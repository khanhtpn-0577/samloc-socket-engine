#pragma once

#include <string>
#include <cstdint>

class ConnectionHandler {
public:
    ConnectionHandler(int clientSocket);
    ~ConnectionHandler();
    
    // Handle client connection (blocking)
    void handle();
    
    // Receive data from client
    std::string receive(size_t bufferSize = 4096);
    
    // Send data to client
    bool send(const std::string& data);
    
    // Check if client is connected
    bool isConnected() const;
    
    // Close connection
    void close();
    
    // Get client socket file descriptor
    int getSocketFd() const { return clientSocket; }

private:
    int clientSocket;
    bool connected;
};
