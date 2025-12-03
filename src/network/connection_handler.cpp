#include "connection_handler.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

#define INVALID_SOCKET -1

// Constructor
ConnectionHandler::ConnectionHandler(int clientSocket)
    : clientSocket(clientSocket), connected(true) {
}

// Destructor
ConnectionHandler::~ConnectionHandler() {
    close();
}

// Receive data from client
std::string ConnectionHandler::receive(size_t bufferSize) {
    if (!connected || clientSocket == INVALID_SOCKET) {
        return "";
    }
    
    char buffer[bufferSize];
    std::memset(buffer, 0, bufferSize);
    
    int received = recv(clientSocket, buffer, bufferSize - 1, 0);
    
    if (received < 0) {
        std::cerr << "Failed to receive data from client\n";
        connected = false;
        return "";
    }
    
    if (received == 0) {
        std::cout << "Client disconnected (fd=" << clientSocket << ")\n";
        connected = false;
        return "";
    }
    
    return std::string(buffer, received);
}

// Send data to client
bool ConnectionHandler::send(const std::string& data) {
    if (!connected || clientSocket == INVALID_SOCKET) {
        return false;
    }
    
    size_t totalSent = 0;
    size_t dataSize = data.size();
    
    while (totalSent < dataSize) {
        int sent = ::send(clientSocket, data.c_str() + totalSent, dataSize - totalSent, 0);
        
        if (sent < 0) {
            std::cerr << "Failed to send data to client\n";
            connected = false;
            return false;
        }
        
        if (sent == 0) {
            std::cerr << "Client closed connection\n";
            connected = false;
            return false;
        }
        
        totalSent += sent;
    }
    
    return true;
}

// Handle client connection
void ConnectionHandler::handle() {
    std::cout << "Handling client (fd=" << clientSocket << ")\n";
    
    while (connected) {
        // Receive data from client
        std::string data = receive();
        
        if (!connected) {
            break;
        }
        
        if (data.empty()) {
            continue;
        }
        
        std::cout << "Received from client (fd=" << clientSocket << "): " << data << "\n";
        
        // Echo back the data
        std::string response = "Echo: " + data;
        if (send(response)) {
            std::cout << "Sent response to client (fd=" << clientSocket << "): " << response << "\n";
        } else {
            break;
        }
    }
    
    std::cout << "Client handler finished (fd=" << clientSocket << ")\n";
}

// Check if connected
bool ConnectionHandler::isConnected() const {
    return connected && (clientSocket != INVALID_SOCKET);
}

// Close connection
void ConnectionHandler::close() {
    if (clientSocket != INVALID_SOCKET) {
        ::close(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    connected = false;
}
