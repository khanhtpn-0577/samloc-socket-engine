#include "rank_logic.h"
#include "../../db/repository/rank_repository.h"
#include <iostream>
#include <cstdint>

RankLogic::RankLogic(Database& db)
    : database_(db) {}

std::string RankLogic::getFriendRanking(uint32_t userId) {
    std::cout << "[RankLogic - Start] Loading friend ranking for userId="
              << userId << "\n";

    std::string payload = R"({"ranks":[)";

    try {
        RankRepository repo(database_);
        auto ranks = repo.getFriendRanking(userId);

        for (size_t i = 0; i < ranks.size(); ++i) {
            payload += "{";
            payload += "\"id\":" + std::to_string(ranks[i].userId) + ",";
            payload += "\"name\":\"" + ranks[i].username + "\",";
            payload += "\"balance\":" + std::to_string(ranks[i].balance);
            payload += "}";

            if (i + 1 < ranks.size()) payload += ",";
        }
    } catch (...) {
        std::cerr << "[RankLogic] Failed to load ranking\n";
    }

    payload += "]}";
    std::cout << "[RankLogic - End] Loaded friend ranking for userId="
              << userId << ": " << payload << "\n";
    return payload;
}
