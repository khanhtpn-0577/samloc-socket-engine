#include "challenge_handler.h"
#include <iostream>
#include <chrono>
#include <cstring>

ChallengeHandler::ChallengeHandler(ChallengeLogic& logic)
    : challengeLogic_(logic) {}

Message ChallengeHandler::handleSendChallenge(
    const Message& incomingMsg
) {
    uint32_t senderId = incomingMsg.header.senderId;

    std::cout << "[ChallengeHandler - Start] Handle SEND_CHALLENGE from userId="
              << senderId << "\n";

    // Delegate toàn bộ parsing + logic cho ChallengeLogic
    std::string payload =
        challengeLogic_.processSendChallenge(senderId, incomingMsg.payload);

    Message response;
    response.header.messageType =
        static_cast<uint16_t>(MessageType::SEND_CHALLENGE_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = payload.size();
    std::memset(response.header.token, 0, 32);
    response.payload = payload;

    std::cout << "[ChallengeHandler - End] Prepared SEND_CHALLENGE_RESPONSE: "
              << payload << "\n";

    return response;
}
