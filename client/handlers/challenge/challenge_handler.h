#pragma once

#include <string>
#include "../../net/protocol.h"
#include "../session/client_session.h"

/**
 * ChallengeHandler
 *  - Handles challenge-related messages from server
 *  - CHALLENGE_NOTIFICATION, SEND_CHALLENGE_RESPONSE, ACCEPT_CHALLENGE_RESPONSE, etc.
 */
class ChallengeHandler {
public:
    explicit ChallengeHandler(ClientSession& session);

    // Handle CHALLENGE_NOTIFICATION (someone challenged you)
    void onChallengeNotification(const Message& message);

    // Handle SEND_CHALLENGE_RESPONSE
    void onSendChallengeResponse(const Message& message);

    // Handle ACCEPT_CHALLENGE_RESPONSE
    void onAcceptChallengeResponse(const Message& message);

    // Handle REJECT_CHALLENGE_RESPONSE
    void onRejectChallengeResponse(const Message& message);

    // Handle CANCEL_CHALLENGE_RESPONSE
    void onCancelChallengeResponse(const Message& message);

    // Handle CHALLENGE_EXPIRED
    void onChallengeExpired(const Message& message);

private:
    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);
    bool parseBoolField(const std::string& payload, const std::string& key);

private:
    ClientSession& session;
};
