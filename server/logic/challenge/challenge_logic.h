#pragma once

#include <string>
#include <cstdint>
#include "../../db/database.h"

struct ChallengeResult {
    bool success;
    std::string message;
    uint32_t roomId;
    uint32_t targetUserId;
    std::string roomType;
    int64_t betAmount;
    std::string senderUsername;
};

class ChallengeLogic {
public:
    explicit ChallengeLogic(Database& db);

    ChallengeResult processSendChallenge(
        uint32_t senderId,
        const std::string& payload
    );

private:
    Database& database_;
};
