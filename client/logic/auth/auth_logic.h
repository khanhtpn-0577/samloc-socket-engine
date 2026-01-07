#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include "../../net/auth/auth_sender.h"

/**
 * AuthLogic (Client-side)
 *  - Handles validation and processing of auth operations
 *  - Works with AuthSender to send auth messages to server
 */
class AuthLogic {
public:
    explicit AuthLogic(AuthSender& authSender);

    using ValidationCallback = std::function<void(bool success, const std::string& message)>;

    // Signup validation and sending
    void onSignupSender(
        const std::string& username,
        const std::string& password,
        const std::string& displayName,
        ValidationCallback callback = nullptr
    );

    void onLoginSender(
        const std::string& username,
        const std::string& password,
        ValidationCallback callback = nullptr
    );

<<<<<<< HEAD
    void getCurrentBalance(uint32_t userId);
=======
    void setValidationCallback(ValidationCallback cb);
>>>>>>> 50ca5d8 (update auth validation checks)

    // Update identity (userId + token) for subsequent auth messages
    void updateIdentity(uint32_t newUserId, const std::string& newToken);

private:
    AuthSender& authSender;
    ValidationCallback validationCallback_;
};
