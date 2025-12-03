#pragma once

#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include "protocol.h"
#include "client_socket.h"

class MessageSender {
public:
    MessageSender(ClientSocket& socket, uint32_t userId, const std::string& token);
    
    // Send direct message
    bool sendDirectMessage(uint32_t receiverId, const std::string& messageContent);
    
    // Send room message
    bool sendRoomMessage(uint32_t roomId, const std::string& messageContent);
    
    // Send login
    bool sendLogin(const std::string& username, const std::string& password);
    
    // Send logout
    bool sendLogout();
    
    // Generic send message
    bool sendMessage(MessageType type, const std::string& payload);
    
    // Get current timestamp
    static uint64_t getCurrentTimestamp();

private:
    ClientSocket& socket;
    uint32_t userId;
    std::string token;
    
    // Create base message with header
    Message createMessage(MessageType type, const std::string& payload);
};
