#pragma once

#include "../../net/protocol.h"
#include "../../logic/auth/auth_logic.h"
#include <string>

/**
 * AuthHandler
 *  - Handles SIGNUP, LOGIN, LOGOUT messages
 *  - Calls AuthLogic for business logic
 *  - Constructs response messages
 */
class AuthHandler {
public:
    explicit AuthHandler(AuthLogic& authLogic);

    // Handle SIGNUP message
    Message handleSignup(const Message& incomingMsg);

    // Handle LOGIN message
    Message handleLogin(const Message& incomingMsg);

    // Handle LOGOUT message
    Message handleLogout(const Message& incomingMsg);

    // Parse uint32 field from JSON-like payload
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);

private:
    // Create response message
    Message createResponse(
        MessageType type,
        bool success,
        const std::string& message,
        uint32_t userId = 0,
        const std::string& token = ""
    );

    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    

    // Build simple JSON-like payload
    std::string buildPayload(
        bool success,
        const std::string& message,
        uint32_t userId = 0,
        const std::string& token = ""
    );

private:
    AuthLogic& authLogic;
};
