#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include "../../net/auth/auth_sender.h"

/**
 * AuthLogic (Client-side)
 *  - Handles validation and processing of auth operations
 *  - Works with AuthSender to send auth messages to server
 */
class AuthLogic {
public:
    explicit AuthLogic(AuthSender& authSender);

    // Signup validation and sending
    void onSignupSender(
        const std::string& username,
        const std::string& password,
        const std::string& displayName
    );

private:
    AuthSender& authSender;
};
