#pragma once

#include "../../net/protocol.h"
#include "../../logic/challenge/challenge_logic.h"
#include "../session/session_manager.h"
#include <string>

// Forward declaration
class ConnectionHandler;

/**
 * ChallengeHandler
 *  - Handles SEND_CHALLENGE, ACCEPT_CHALLENGE, REJECT_CHALLENGE, CANCEL_CHALLENGE messages
 *  - Calls ChallengeLogic for business logic
 *  - Sends CHALLENGE_NOTIFICATION to receiver when challenge is sent
 *  - Constructs response messages
 */
class ChallengeHandler {
public:
    explicit ChallengeHandler(ChallengeLogic& challengeLogic);

    // Handle SEND_CHALLENGE message
    Message handleSendChallenge(const Message& incomingMsg);

    // Handle ACCEPT_CHALLENGE message
    Message handleAcceptChallenge(const Message& incomingMsg);

    // Handle REJECT_CHALLENGE message
    Message handleRejectChallenge(const Message& incomingMsg);

    // Handle CANCEL_CHALLENGE message
    Message handleCancelChallenge(const Message& incomingMsg);

private:
    // Create response message
    Message createResponse(
        MessageType type,
        bool success,
        const std::string& message,
        uint32_t challengeId = 0
    );

    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);

    // Build simple JSON-like payload
    std::string buildPayload(
        bool success,
        const std::string& message,
        uint32_t challengeId = 0,
        uint32_t senderId = 0,
        uint32_t receiverId = 0
    );

    // Send notification to receiver
    void notifyReceiver(uint32_t receiverId, uint32_t senderId, uint32_t challengeId);

private:
    ChallengeLogic& challengeLogic;
};
