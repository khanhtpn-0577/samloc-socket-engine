#pragma once

#include <string>
#include <cstdint>
#include "../protocol.h"
#include "../client_socket.h"

class ChallengeSender {
public:
    ChallengeSender(ClientSocket& socket, uint32_t userId, const std::string& token);

    bool sendChallenge(uint32_t receiverId);
    bool acceptChallenge(uint32_t challengeId);
    bool rejectChallenge(uint32_t challengeId);
    bool cancelChallenge(uint32_t challengeId);

    void updateIdentity(uint32_t newUserId, const std::string& newToken);

private:
    ClientSocket& socket;
    uint32_t userId;
    std::string token;

    static uint64_t getCurrentTimestamp();
    Message createMessage(MessageType type, const std::string& payload);
    bool sendMessage(MessageType type, const std::string& payload);
};
