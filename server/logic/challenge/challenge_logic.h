#pragma once

#include <string>
#include <cstdint>
#include "../../db/database.h"

class ChallengeLogic {
public:
    explicit ChallengeLogic(Database& db);

    // xử lý SEND_CHALLENGE
    std::string processSendChallenge(
        uint32_t senderId,
        const std::string& payload
    );

private:
    Database& database_;
};
