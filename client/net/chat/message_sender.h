#pragma once

#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include "../protocol.h"
#include "../client_socket.h"
#include "../../handlers/session/client_session.h"

class MessageSender {
public:
    MessageSender(ClientSocket& socket, ClientSession& session);
    
    // Send direct message
    bool sendDirectMessage(uint32_t receiverId, const std::string& messageContent);
    
    // Send room message
    bool sendRoomMessage(uint32_t roomId, const std::string& messageContent);
    
    // Generic send message
    bool sendMessage(MessageType type, const std::string& payload);
    
    // Get current timestamp
    static uint64_t getCurrentTimestamp();

    void updateIdentity(uint32_t newUserId, const std::string& newToken);

private:
    ClientSocket& socket;
    ClientSession& session;
    
    // Create base message with header
    Message createMessage(MessageType type, const std::string& payload);
};
