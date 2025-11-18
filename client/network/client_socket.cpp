#include "client_socket.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define CLOSE_SOCKET close
#define INVALID_SOCKET -1

// Constructor
ClientSocket::ClientSocket(const std::string& serverIp, int port)
    : serverIp(serverIp), port(port), socketFd(INVALID_SOCKET), connected(false) {
}

// Destructor
ClientSocket::~ClientSocket() {
    disconnect();
}

// Create socket
bool ClientSocket::createSocket() {
    socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketFd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    return true;
}

// Connect to server
bool ClientSocket::connect() {
    if (!createSocket()) {
        return false;
    }
    
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    // Convert IP address
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address: " << serverIp << "\n";
        return false;
    }
    
    // Connect to server
    if (::connect(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection to server failed: " << serverIp << ":" << port << "\n";
        CLOSE_SOCKET(socketFd);
        socketFd = INVALID_SOCKET;
        return false;
    }
    
    connected = true;
    std::cout << "Connected to server: " << serverIp << ":" << port << "\n";
    return true;
}

// Disconnect from server
void ClientSocket::disconnect() {
    if (socketFd != INVALID_SOCKET) {
        CLOSE_SOCKET(socketFd);
        socketFd = INVALID_SOCKET;
    }
    connected = false;
    std::cout << "Disconnected from server\n";
}

// Send raw data
bool ClientSocket::send(const std::string& data) {
    if (!connected || socketFd == INVALID_SOCKET) {
        std::cerr << "Not connected to server\n";
        return false;
    }
    
    size_t totalSent = 0;
    size_t dataSize = data.size();
    
    while (totalSent < dataSize) {
        int sent = ::send(socketFd, data.c_str() + totalSent, dataSize - totalSent, 0);
        
        if (sent < 0) {
            std::cerr << "Failed to send data\n";
            disconnect();
            return false;
        }
        
        if (sent == 0) {
            std::cerr << "Server closed connection\n";
            disconnect();
            return false;
        }
        
        totalSent += sent;
    }
    
    return true;
}

// Receive data
std::string ClientSocket::receive(size_t bufferSize) {
    if (!connected || socketFd == INVALID_SOCKET) {
        std::cerr << "Not connected to server\n";
        return "";
    }
    
    char buffer[bufferSize];
    std::memset(buffer, 0, bufferSize);
    
    int received = recv(socketFd, buffer, bufferSize - 1, 0);
    
    if (received < 0) {
        std::cerr << "Failed to receive data\n";
        disconnect();
        return "";
    }
    
    if (received == 0) {
        std::cerr << "Server closed connection\n";
        disconnect();
        return "";
    }
    
    return std::string(buffer, received);
}

// Send message (serialized)
bool ClientSocket::sendMessage(const Message& message) {
    std::string serialized = message.serialize();
    return send(serialized);
}

// Receive message
Message ClientSocket::receiveMessage() {
    // First, receive header (50 bytes)
    std::string headerData;
    while (headerData.size() < sizeof(MessageHeader)) {
        std::string chunk = receive(sizeof(MessageHeader) - headerData.size());
        if (chunk.empty()) {
            return Message();  // Connection error
        }
        headerData += chunk;
    }
    
    // Parse header to get payload length
    MessageHeader header;
    std::memcpy(&header, headerData.data(), sizeof(MessageHeader));
    
    // Receive payload
    std::string payloadData;
    uint32_t payloadLength = header.payloadLength;
    
    while (payloadData.size() < payloadLength) {
        size_t remainingBytes = payloadLength - payloadData.size();
        std::string chunk = receive(remainingBytes);
        if (chunk.empty() && payloadLength > 0) {
            return Message();  // Connection error
        }
        payloadData += chunk;
    }
    
    // Combine header and payload
    std::string completeData = headerData + payloadData;
    
    // Deserialize
    return Message::deserialize(completeData);
}

// Check if connected
bool ClientSocket::isConnected() const {
    return connected && (socketFd != INVALID_SOCKET);
}
