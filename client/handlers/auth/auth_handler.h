#pragma once

#include <string>
#include <functional>
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

    void onSignupSender(const std::string& username, const std::string& password, const std::string& displayName);

    void onLoginSender(const std::string& username, const std::string& password);

    void getCurrentBalance();

    // Handle SIGNUP_RESPONSE
    void onSignupResponse(const Message& message);

    // Handle LOGIN_RESPONSE
    void onLoginResponse(const Message& message);

    // Handle LOGOUT_RESPONSE
    void onLogoutResponse(const Message& message);

    void onBalanceResponse(const Message& message);

    using SignupCallback = std::function<void(
        bool success,
        uint32_t userId,
        const std::string& username,
        const std::string& displayName,
        const std::string& message
    )>;

    void setSignupCallback(SignupCallback cb);

    using LoginCallback = std::function<void(
        bool success,
        uint32_t userId,
        const std::string& token,
        const std::string& message
    )>;

    void setLoginCallback(LoginCallback cb);

    using LogoutCallback = std::function<void(
        bool success,
        const std::string& message
    )>;

    void setLogoutCallback(LogoutCallback cb);


private:
    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);
    bool parseBoolField(const std::string& payload, const std::string& key);

private:
    AuthLogic& authLogic_;
    ClientSession& session_;

    SignupCallback signupCallback_;
    LoginCallback loginCallback_;
    LogoutCallback logoutCallback_;

};
