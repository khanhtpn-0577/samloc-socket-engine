#include "rank_handler.h"
#include <iostream>
#include <chrono>
#include <cstring>

RankHandler::RankHandler(RankLogic& logic)
    : rankLogic_(logic) {}

Message RankHandler::handleFriendRankRequest(
    const Message& incomingMsg
) {
    uint32_t userId = incomingMsg.header.senderId;

    std::cout << "[Rank Handler - Start] Handle FRIEND_RANK_REQUEST for userId="
              << userId << "\n";

    std::string payload = rankLogic_.getFriendRanking(userId);

    Message response;
    response.header.messageType =
        static_cast<uint16_t>(MessageType::FRIEND_RANK_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = payload.size();
    std::memset(response.header.token, 0, 32);
    response.payload = payload;
    std::cout << "[Rank Handler - End] Prepared FRIEND_RANK_RESPONSE for userId="
              << userId << " with payload: " << payload << "\n";
    return response;
}
