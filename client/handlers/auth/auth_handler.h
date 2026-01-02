#pragma once

#include <string>
#include "../../net/protocol.h"
#include "../session/client_session.h"
#include "../../logic/auth/auth_logic.h"
#include "../../net/auth/auth_sender.h"

/**
 * AuthHandler
 *  - Handles auth response messages from server (SIGNUP_RESPONSE, LOGIN_RESPONSE, LOGOUT_RESPONSE)
 *  - Updates client session state
 */
class AuthHandler {
public:
    AuthHandler(AuthLogic& logic, ClientSession& session);

    // TODO: thêm đoạn giữa sender và GUI 
    void onSignupSender(const std::string& username, const std::string& password, const std::string& displayName);

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
    AuthLogic& authLogic_;
    ClientSession& session_;
};
