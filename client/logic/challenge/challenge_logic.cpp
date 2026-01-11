#include "challenge_logic.h"
#include <iostream>

ChallengeLogic::ChallengeLogic(ChallengeSender& challengeSender)
    : challengeSender_(challengeSender) {}

void ChallengeLogic::sendChallenge(uint32_t targetUserId, uint32_t roomId) {
    if (roomId == 0 || targetUserId == 0) {
        std::cerr << "[ChallengeLogic] Invalid roomID:"
                  << roomId << ", target=" << targetUserId << "\n";
        return;
    }

    // Payload đơn giản, rõ ràng, dễ debug
    std::string payload =
        "{"
        "\"roomId\":" + std::to_string(roomId) + ","
        "\"targetUserId\":" + std::to_string(targetUserId) +
        "}";

    if (!challengeSender_.sendMessage(
            MessageType::SEND_CHALLENGE,
            payload
        )) {
        std::cerr << "[ChallengeLogic] Failed to send challenge request\n";
    } else {
        std::cout << "[ChallengeLogic] Challenge request sent: roomId="
                  << roomId << " -> targetUserId="
                  << targetUserId << "\n";
    }
}
