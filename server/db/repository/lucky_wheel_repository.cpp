#include "lucky_wheel_repository.h"
#include <iostream>

LuckyWheelRepository::LuckyWheelRepository(Database& db)
    : database_(db) {}

long long LuckyWheelRepository::getPlayerBalance(uint32_t userId) {
    std::string sql =
        "SELECT balance FROM players WHERE player_id = ?";

    QueryResult rows =
        database_.queryPrepared(sql, { std::to_string(userId) });

    if (rows.empty()) {
        throw std::runtime_error("Player not found");
    }

    return std::stoll(rows[0].at("balance"));
}

bool LuckyWheelRepository::updatePlayerBalance(
    uint32_t userId,
    long long newBalance
) {
    std::string sql =
        "UPDATE players SET balance = ? WHERE player_id = ?";

    database_.executePrepared(
        sql,
        { std::to_string(newBalance), std::to_string(userId) }
    );

    return true;
}

std::vector<LuckySpinReward>
LuckyWheelRepository::getAllRewards() {
    std::vector<LuckySpinReward> result;

    std::string sql =
        "SELECT reward_id, reward_name, reward_type, reward_amount, probability "
        "FROM lucky_spin_rewards";

    QueryResult rows = database_.query(sql);

    for (const auto& row : rows) {
        LuckySpinReward r;
        r.rewardId     = std::stoul(row.at("reward_id"));
        r.rewardName   = row.at("reward_name");
        r.rewardType   = row.at("reward_type");
        r.rewardAmount = std::stoll(row.at("reward_amount"));
        r.probability  = std::stod(row.at("probability"));
        result.push_back(r);
    }

    return result;
}
