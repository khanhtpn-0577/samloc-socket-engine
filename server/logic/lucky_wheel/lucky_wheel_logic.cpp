#include "lucky_wheel_logic.h"
#include "../../db/repository/lucky_wheel_repository.h"
#include <iostream>
#include <random>

LuckyWheelLogic::LuckyWheelLogic(Database& db)
    : database_(db) {}

std::string LuckyWheelLogic::spin(uint32_t userId) {
    std::cout << "[LuckyWheelLogic - Start] Spin for userId="
              << userId << "\n";

    LuckyWheelRepository repo(database_);

    try {
        long long balance = repo.getPlayerBalance(userId);

        // ==== CHECK COST ====
        constexpr long long SPIN_COST = 50000;

        if (balance < SPIN_COST) {
            return R"({
                "success": false,
                "message": "Not enough balance",
                "reward": null,
                "newBalance": )" + std::to_string(balance) + "}";
        }

        // ==== LOAD REWARDS ====
        auto rewards = repo.getAllRewards();
        if (rewards.empty()) {
            throw std::runtime_error("No rewards configured");
        }

        // ==== BUILD PROBABILITY VECTOR ====
        std::vector<double> probabilities;
        for (const auto& r : rewards) {
            probabilities.push_back(r.probability);
        }

        int rewardIndex = rollRewardIndex(probabilities);
        const auto& reward = rewards[rewardIndex];

        // ==== UPDATE BALANCE ====
        long long newBalance =
            balance - SPIN_COST + reward.rewardAmount;

        repo.updatePlayerBalance(userId, newBalance);

        // ==== BUILD PAYLOAD ====
        std::string payload = "{";
        payload += "\"success\":true,";
        payload += "\"message\":\"Spin successful\",";
        payload += "\"reward\":{";
        payload += "\"name\":\"" + reward.rewardName + "\",";
        payload += "\"amount\":" + std::to_string(reward.rewardAmount);
        payload += "},";
        payload += "\"newBalance\":" + std::to_string(newBalance);
        payload += "}";

        std::cout << "[LuckyWheelLogic - End] userId=" << userId
                  << ", reward=" << reward.rewardName
                  << ", newBalance=" << newBalance << "\n";

        return payload;

    } catch (const std::exception& e) {
        std::cerr << "[LuckyWheelLogic] Error: " << e.what() << "\n";

        return R"({
            "success": false,
            "message": "Internal server error",
            "reward": null,
            "newBalance": 0
        })";
    }
}

int LuckyWheelLogic::rollRewardIndex(
    const std::vector<double>& probabilities
) {
    if (probabilities.empty()) {
        return 0;
    }

    // Random engine
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // Phân phối rời rạc theo trọng số
    std::discrete_distribution<> dist(
        probabilities.begin(),
        probabilities.end()
    );

    return dist(gen);
}

