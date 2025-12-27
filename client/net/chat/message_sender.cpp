#include "message_sender.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <sstream>
#ifdef _WIN32
    #include <winsock2.h> // Dành cho Windows (để VS Code không báo lỗi)
#else
    #include <arpa/inet.h> // Dành cho Linux/WSL
#endif
// Constructor
MessageSender::MessageSender(ClientSocket& socket, uint32_t userId, const std::string& token)
    : socket(socket), userId(userId), token(token) {
}

// Get current timestamp (milliseconds since epoch)
uint64_t MessageSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Create base message with header
Message MessageSender::createMessage(MessageType type, const std::string& payload) {
    Message msg;
    
    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId = userId;
    msg.header.timestamp = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();
    
    // Set token
    setToken(msg.header, token);
    
    msg.payload = payload;
    
    return msg;
}

// Send direct message
bool MessageSender::sendDirectMessage(uint32_t receiverId, const std::string& messageContent) {
    // Create payload: receiverId (4 bytes) + message content
    std::string payload;
    
    // Add receiver ID (4 bytes, network byte order)
    uint32_t networkReceiverId = htonl(receiverId);
    payload.append(reinterpret_cast<const char*>(&networkReceiverId), sizeof(uint32_t));
    
    // Add message content
    payload.append(messageContent);
    
    Message msg = createMessage(MessageType::CHAT_DIRECT, payload);
    
    if (socket.sendMessage(msg)) {
        std::cout << "[Client: message_sender: Direct message sent to server to delivery to user " << receiverId << ": " << messageContent << "\n";
        return true;
    } else {
        std::cerr << "Failed to send direct message\n";
        return false;
    }
}

// Send room message
bool MessageSender::sendRoomMessage(uint32_t roomId, const std::string& messageContent) {
    // Create payload: roomId (4 bytes) + message content
    std::string payload;
    
    // Add room ID (4 bytes, network byte order)
    uint32_t networkRoomId = htonl(roomId);
    payload.append(reinterpret_cast<const char*>(&networkRoomId), sizeof(uint32_t));
    
    // Add message content
    payload.append(messageContent);
    
    Message msg = createMessage(MessageType::CHAT_ROOM, payload);
    
    if (socket.sendMessage(msg)) {
        std::cout << "Room message sent to room " << roomId << ": " << messageContent << "\n";
        return true;
    } else {
        std::cerr << "Failed to send room message\n";
        return false;
    }
}


// Generic send message
bool MessageSender::sendMessage(MessageType type, const std::string& payload) {
    Message msg = createMessage(type, payload);
    
    if (socket.sendMessage(msg)) {
        std::cout << "Message sent (type: " << static_cast<uint16_t>(type) << ")\n";
        return true;
    } else {
        std::cerr << "Failed to send message\n";
        return false;
    }
}
