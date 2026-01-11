#include "challenge_logic.h"
#include "../../db/repository/challenge_repository.h"
#include <iostream>

ChallengeLogic::ChallengeLogic(Database& db)
    : database_(db) {}

ChallengeResult ChallengeLogic::processSendChallenge(
    uint32_t senderId,
    const std::string& payload
) {
    ChallengeResult result{};
    result.success = false;
    result.roomId = 0;
    result.targetUserId = 0;

    size_t roomPos = payload.find("\"roomId\":");
    size_t targetPos = payload.find("\"targetUserId\":");

    if (roomPos == std::string::npos ||
        targetPos == std::string::npos) {
        result.message = "Invalid payload";
        return result;
    }

    result.roomId = std::stoul(
        payload.substr(roomPos + 9,
        payload.find(",", roomPos) - (roomPos + 9))
    );

    result.targetUserId = std::stoul(
        payload.substr(targetPos + 15)
    );

    std::cout << "[ChallengeLogic] senderId=" << senderId
              << " roomId=" << result.roomId
              << " targetUserId=" << result.targetUserId << "\n";

    ChallengeRepository repo(database_);

    if (!repo.isRoomOwner(senderId, result.roomId)) {
        result.message = "You are not the room owner";
        return result;
    }

    //Load room info
    RoomChallengeInfo roomInfo;
    if (!repo.getRoomChallengeInfo(result.roomId, roomInfo)) {
        result.message = "Room not found";
        return result;
    }

    if (!repo.getUsernameByUserId(senderId, result.senderUsername)) {
        result.message = "Sender not found";
        return result;
    }

    result.roomType  = roomInfo.roomType;
    result.betAmount = roomInfo.betAmount;

    result.success = true;
    result.message = "Challenge sent successfully";
    return result;
}
