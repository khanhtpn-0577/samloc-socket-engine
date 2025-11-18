#include "message_sender.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <sstream>

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
        std::cout << "Direct message sent to user " << receiverId << ": " << messageContent << "\n";
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

// Send login
bool MessageSender::sendLogin(const std::string& username, const std::string& password) {
    // Create payload: username length + username + password length + password
    std::string payload;
    
    // Add username
    uint32_t usernameLen = username.size();
    payload.append(reinterpret_cast<const char*>(&usernameLen), sizeof(uint32_t));
    payload.append(username);
    
    // Add password
    uint32_t passwordLen = password.size();
    payload.append(reinterpret_cast<const char*>(&passwordLen), sizeof(uint32_t));
    payload.append(password);
    
    Message msg = createMessage(MessageType::LOGIN, payload);
    
    if (socket.sendMessage(msg)) {
        std::cout << "Login request sent for user: " << username << "\n";
        return true;
    } else {
        std::cerr << "Failed to send login\n";
        return false;
    }
}

// Send logout
bool MessageSender::sendLogout() {
    Message msg = createMessage(MessageType::LOGOUT, "");
    
    if (socket.sendMessage(msg)) {
        std::cout << "Logout request sent\n";
        return true;
    } else {
        std::cerr << "Failed to send logout\n";
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
