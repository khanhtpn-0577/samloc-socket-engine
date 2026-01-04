#pragma once
#include <string>
#include "../../db/database.h"
#include <cstdint>


class RankLogic {
public:
    explicit RankLogic(Database& db);

    std::string getFriendRanking(uint32_t userId);

private:
    Database& database_;
};
