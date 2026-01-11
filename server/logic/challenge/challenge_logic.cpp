#include "challenge_logic.h"
#include "../../db/repository/challenge_repository.h"
#include <iostream>

ChallengeLogic::ChallengeLogic(Database& db)
    : database_(db) {}

std::string ChallengeLogic::processSendChallenge(
    uint32_t senderId,
    const std::string& payload
) {
    uint32_t roomId = 0;
    uint32_t targetUserId = 0;

    // Parse payload rất đơn giản (giống phong cách Rank)
    size_t roomPos = payload.find("\"roomId\":");
    size_t targetPos = payload.find("\"targetUserId\":");

    if (roomPos == std::string::npos ||
        targetPos == std::string::npos) {
        return R"({"success":0,"message":"Invalid payload"})";
    }

    roomId = std::stoul(
        payload.substr(roomPos + 9,
        payload.find(",", roomPos) - (roomPos + 9))
    );

    targetUserId = std::stoul(
        payload.substr(targetPos + 15)
    );

    std::cout << "[ChallengeLogic] senderId=" << senderId
              << " roomId=" << roomId
              << " targetUserId=" << targetUserId << "\n";

    ChallengeRepository repo(database_);

    // CHECK: sender có phải chủ phòng không
    if (!repo.isRoomOwner(senderId, roomId)) {
        return R"({"success":0,"message":"You are not the room owner"})";
    }

    // OK
    return R"({"success":1,"message":"Challenge sent successfully"})";
}
