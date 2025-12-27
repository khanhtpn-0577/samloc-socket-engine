#pragma once

#include <string>
#include "../../net/protocol.h"
#include "../session/client_session.h"

/**
 * AuthHandler
 *  - Handles auth response messages from server (SIGNUP_RESPONSE, LOGIN_RESPONSE, LOGOUT_RESPONSE)
 *  - Updates client session state
 */
class AuthHandler {
public:
    explicit AuthHandler(ClientSession& session);

    // Handle SIGNUP_RESPONSE
    void onSignupResponse(const Message& message);

    // Handle LOGIN_RESPONSE
    void onLoginResponse(const Message& message);

    // Handle LOGOUT_RESPONSE
    void onLogoutResponse(const Message& message);

private:
    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);
    bool parseBoolField(const std::string& payload, const std::string& key);

private:
    ClientSession& session;
};
