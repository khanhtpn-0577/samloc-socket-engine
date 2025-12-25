#pragma once

#include <string>
#include <cstdint>
#include "protocol.h"

class ClientSocket {
public:
    ClientSocket(const std::string& serverIp, int port); // constructor
    ~ClientSocket(); // destructor
    
    // Connect to server
    bool connect();
    
    // Disconnect from server
    void disconnect();
    
    // Send raw data
    bool send(const std::string& data);
    
    // Receive data with size
    bool receive(void* buffer, size_t size);

    // Send message (serialized)
    bool sendMessage(const Message& message);
    
    // Receive message
    Message receiveMessage();
    
    // Check if connected
    bool isConnected() const;
    
    // Get socket file descriptor
    int getSocketFd() const { return socketFd; }

private:
    std::string serverIp;
    int port; // port cua server
    int socketFd; //File descriptor của socket (giá trị int)
    bool connected;
    
    // Helper: Create socket
    bool createSocket();
};
