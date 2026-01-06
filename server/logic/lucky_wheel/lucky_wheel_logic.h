#pragma once

#include <string>
#include <cstdint>
#include "../../db/database.h"

class LuckyWheelLogic {
public:
    explicit LuckyWheelLogic(Database& db);

    std::string spin(uint32_t userId);

private:
    Database& database_;

    int rollRewardIndex(const std::vector<double>& probabilities);
};
