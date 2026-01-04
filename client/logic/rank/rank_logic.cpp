#include "rank_logic.h"
#include <iostream>

RankLogic::RankLogic(RankSender& rankSender)
    : rankSender_(rankSender) {}

void RankLogic::requestFriendRank(uint32_t userId) {
    if (userId == 0) {
        std::cerr << "[RankLogic] Invalid userId\n";
        return;
    }

    std::string payload = "{\"userId\":" + std::to_string(userId) + "}";

    if (!rankSender_.sendMessage(
            MessageType::FRIEND_RANK_REQUEST,
            payload
        )) {
        std::cerr << "[RankLogic] Failed to send friend rank request\n";
    } else {
        std::cout << "[RankLogic] Friend rank request sent for userId="
                  << userId << "\n";
    }
}
