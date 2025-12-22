#include "client_socket.h"
#include <iostream>
#include <cstring>
#include <vector>
#ifdef _WIN32
    // --- Dành cho Windows ---
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <io.h>      // Thay thế cho unistd.h trên Windows
    
    // Định nghĩa lại một số hàm/macro để giống Linux
    #define close closesocket
    #pragma comment(lib, "ws2_32.lib") // Tự động link thư viện socket trên Windows
#else
    // --- Dành cho Linux / WSL ---
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> // Chứa hàm close()
#endif

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
        std::cerr << "[client_socket]Failed to create socket\n";
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
        std::cerr << "[client_socket]Invalid server IP address: " << serverIp << "\n";
        return false;
    }
    
    // Connect to server
    if (::connect(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[client_socket]Connection to server failed: " << serverIp << ":" << port << "\n";
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
    std::cout << "[client_socket]Disconnected from server\n";
}

// Send raw data
bool ClientSocket::send(const std::string& data) {
    if (!connected || socketFd == INVALID_SOCKET) {
        std::cerr << "[client_socket]Not connected to server\n";
        return false;
    }
    
    size_t totalSent = 0;
    size_t dataSize = data.size();
    
    while (totalSent < dataSize) {
        int sent = ::send(socketFd, data.c_str() + totalSent, dataSize - totalSent, 0); //dataSize - totalSent: so byte con lai de gui
        
        if (sent < 0) {
            std::cerr << "[client_socket]Failed to send data\n";
            disconnect();
            return false;
        }
        
        if (sent == 0) {
            std::cerr << "[client_socket]sent = 0: Server closed connection\n";
            disconnect();
            return false;
        }
        
        totalSent += sent;
    }
    
    return true;
}

// Receive data
bool ClientSocket::receive(void* buffer, size_t size) {
    char* buf = static_cast<char*>(buffer);
    size_t received = 0;

    while (received < size) {
        ssize_t r = ::recv(socketFd, buf + received, size - received, 0);

        if (r > 0) {
            received += static_cast<size_t>(r);
            continue;
        }

        if (r == 0) {
            // peer closed connection
            std::cerr << "[client_socket] recv EOF (server closed)\n";
            disconnect();
            return false;
        }

        // r < 0
        if (errno == EINTR) {
            continue; // retry
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue; // retry (blocking socket hiếm khi vào đây)
        }

        std::cerr << "[client_socket] recv error errno="
                  << errno << " (" << std::strerror(errno) << ")\n";
        disconnect();
        return false;
    }

    return true;
}


// Send message (serialized)
bool ClientSocket::sendMessage(const Message& message) {
    std::string serialized = message.serialize();
    return send(serialized);
}

// Receive message
Message ClientSocket::receiveMessage() {
    MessageHeader header;

    // ===== Read header exactly =====
    if (!receive(&header, sizeof(MessageHeader))) {
        return Message(); // connection closed or error
    }

    // ===== Read payload exactly =====
    std::string payload;
    if (header.payloadLength > 0) {
        payload.resize(header.payloadLength);

        if (!receive(payload.data(), header.payloadLength)) {
            return Message(); // connection closed or error
        }
    }

    // ===== Build message =====
    Message msg;
    msg.header = header;
    msg.payload = payload;

    return msg;
}


// Check if connected
bool ClientSocket::isConnected() const {
    return connected && (socketFd != INVALID_SOCKET);
}
