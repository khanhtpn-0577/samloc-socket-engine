#pragma once
#include "../../db/database.h"
#include <vector>
#include <string>
#include <cstdint>


struct RankInfo {
    uint32_t userId;
    std::string username;
    long long balance;
};

class RankRepository {
public:
    explicit RankRepository(Database& db);

    std::vector<RankInfo> getFriendRanking(uint32_t userId);

private:
    Database& database_;
};
