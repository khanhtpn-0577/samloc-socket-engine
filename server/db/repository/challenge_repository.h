#pragma once

#include "../../db/database.h"
#include <cstdint>
#include <string>


struct RoomChallengeInfo {
    std::string roomType;
    int64_t betAmount;
};

class ChallengeRepository {
public:
    explicit ChallengeRepository(Database& db);

    bool isRoomOwner(uint32_t userId, uint32_t roomId);
    bool getRoomChallengeInfo(uint32_t roomId, RoomChallengeInfo& out);
    bool getUsernameByUserId(uint32_t userId, std::string& username);
private:
    Database& database_;
};
