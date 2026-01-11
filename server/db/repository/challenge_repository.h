#pragma once

#include "../../db/database.h"
#include <cstdint>

class ChallengeRepository {
public:
    explicit ChallengeRepository(Database& db);

    bool isRoomOwner(uint32_t userId, uint32_t roomId);

private:
    Database& database_;
};
