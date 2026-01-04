#include "rank_repository.h"
#include <cstdint>


RankRepository::RankRepository(Database& db)
    : database_(db) {}

std::vector<RankInfo>
RankRepository::getFriendRanking(uint32_t userId) {
    std::vector<RankInfo> result;

    std::string sql =
        "SELECT p.player_id, p.username, p.balance "
        "FROM friends f "
        "JOIN players p ON p.player_id = f.friend_id "
        "WHERE f.player_id = ? "
        "ORDER BY p.balance DESC;";

    QueryResult rows =
        database_.queryPrepared(sql, { std::to_string(userId) });

    for (const auto& row : rows) {
        RankInfo r;
        r.userId   = std::stoul(row.at("player_id"));
        r.username = row.at("username");
        r.balance  = std::stoll(row.at("balance"));
        result.push_back(r);
    }

    return result;
}
