#pragma once

#include "../../db/database.h"
#include <string>
#include <vector>
#include <cstdint>

struct LuckySpinReward {
    uint32_t rewardId;
    std::string rewardName;
    std::string rewardType;   // coin
    long long rewardAmount;
    double probability;       // %
};

class LuckyWheelRepository {
public:
    explicit LuckyWheelRepository(Database& db);

    long long getPlayerBalance(uint32_t userId);
    bool updatePlayerBalance(uint32_t userId, long long newBalance);

    std::vector<LuckySpinReward> getAllRewards();

private:
    Database& database_;
};
