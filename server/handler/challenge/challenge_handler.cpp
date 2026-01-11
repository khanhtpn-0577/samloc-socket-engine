#include "challenge_handler.h"

#include "../connection/connection_handler.h"
#include "../session/session_manager.h"

#include <iostream>
#include <chrono>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ChallengeHandler::ChallengeHandler(ChallengeLogic& logic)
    : challengeLogic_(logic) {}

Message ChallengeHandler::handleSendChallenge(
    const Message& incomingMsg
) {
    uint32_t senderId = incomingMsg.header.senderId;

    std::cout << "[ChallengeHandler] SEND_CHALLENGE from userId="
              << senderId << "\n";

    // ===== 1. Logic xử lý =====
    ChallengeResult result =
        challengeLogic_.processSendChallenge(
            senderId,
            incomingMsg.payload
        );

    // ===== 2. Response cho sender =====
    Message response;
    response.header.messageType =
        (uint16_t)MessageType::SEND_CHALLENGE_RESPONSE;
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

    response.payload = json({
        {"success", result.success},
        {"message", result.message}
    }).dump();

    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);

    // ===== 3. Nếu OK → notify target user =====
    if (result.success) {
        Message notify;
        notify.header.messageType =
            (uint16_t)MessageType::CHALLENGE_NOTIFICATION;
        notify.header.senderId = 0;
        notify.header.timestamp = response.header.timestamp;

        notify.payload = json({
            {"fromUserId", senderId},
            {"fromUsername", result.senderUsername},
            {"roomId", result.roomId},
            {"roomType", result.roomType},
            {"betAmount", result.betAmount}
        }).dump();

        notify.header.payloadLength = notify.payload.size();
        std::memset(notify.header.token, 0, 32);

        if (auto* c = SessionManager::instance().get(result.targetUserId)) {
            c->sendMessage(notify);
            std::cout << "[ChallengeHandler] Notify sent to targetUserId="
                      << result.targetUserId
                      << " roomId=" << result.roomId << "\n";
        }
    }

    return response;
}
