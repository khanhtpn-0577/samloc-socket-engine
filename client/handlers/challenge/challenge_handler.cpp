#include "challenge_handler.h"
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

ChallengeHandler::ChallengeHandler(
    ChallengeLogic& challengeLogic,
    ClientSession& session
)
    : session_(session),
      challengeLogic_(challengeLogic) {}

void ChallengeHandler::onSendChallenge(uint32_t targetUserId, uint32_t roomId) {
    std::cout << "[ChallengeHandler] Request send challenge to userId="
              << targetUserId << "\n";

    if (!session_.isLoggedIn()) {
        std::cerr << "[ChallengeHandler] Not logged in, ignore send challenge\n";
        return;
    }

    challengeLogic_.sendChallenge(
        targetUserId,
        roomId
    );
}

void ChallengeHandler::setChallengeResultCallback(
    std::function<void(bool, const std::string&)> cb
) {
    challengeResultCallback_ = std::move(cb);
}

void ChallengeHandler::onSendChallengeResponse(
    const Message& message
) {
    bool success = false;
    std::string msg;

    try {
        auto j = json::parse(message.payload);
        success = j.value("success", false);
        msg = j.value("message", "");
    } catch (...) {
        msg = "Invalid response";
    }

    std::cout << "[ChallengeHandler] Response success="
              << success << " message=" << msg << "\n";

    if (challengeResultCallback_) {
        challengeResultCallback_(success, msg);
    }
}


void ChallengeHandler::setChallengeNotifyCallback(
    std::function<void(
        uint32_t,
        const std::string&,
        uint32_t,
        const std::string&,
        int64_t
    )> cb
) {
    challengeNotifyCallback_ = std::move(cb);
}

void ChallengeHandler::onChallengeNotification(
    const Message& message
) {
    try {
        auto j = json::parse(message.payload);

        uint32_t fromUserId = j.value("fromUserId", 0);
        std::string fromUsername = j.value("fromUsername", "");
        uint32_t roomId = j.value("roomId", 0);
        std::string roomType = j.value("roomType", "");
        int64_t betAmount = j.value("betAmount", 0);

        std::cout
            << "[ChallengeHandler] CHALLENGE_NOTIFICATION"
            << " from=" << fromUsername
            << " roomId=" << roomId
            << " bet=" << betAmount << "\n";

        if (challengeNotifyCallback_) {
            challengeNotifyCallback_(
                fromUserId,
                fromUsername,
                roomId,
                roomType,
                betAmount
            );
        }
    } catch (...) {
        std::cerr << "[ChallengeHandler] Invalid CHALLENGE_NOTIFICATION payload\n";
    }
}


