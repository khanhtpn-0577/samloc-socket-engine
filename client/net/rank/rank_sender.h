#pragma once

#include <string>
#include <cstdint>
#include <queue>
#include <mutex>
#include "../protocol.h"
#include "../client_socket.h"
#include "../../handlers/session/client_session.h"

class RankSender {
public:
    RankSender(ClientSocket& socket, ClientSession& session);
    
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
