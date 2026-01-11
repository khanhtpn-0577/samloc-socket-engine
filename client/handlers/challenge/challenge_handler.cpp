#include "challenge_handler.h"
#include <iostream>

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
    const std::string& payload = message.payload;

    bool success = false;
    std::string msg;

    // Parse "success"
    size_t sPos = payload.find("\"success\":");
    if (sPos != std::string::npos) {
        size_t vPos = sPos + 10;
        success = (payload[vPos] == '1');
    }

    // Parse "message"
    size_t mPos = payload.find("\"message\":\"");
    if (mPos != std::string::npos) {
        size_t start = mPos + 11;
        size_t end = payload.find("\"", start);
        msg = payload.substr(start, end - start);
    }

    std::cout << "[ChallengeHandler] Response success="
              << success << " message=" << msg << "\n";

    if (challengeResultCallback_) {
        challengeResultCallback_(success, msg);
    }
}


