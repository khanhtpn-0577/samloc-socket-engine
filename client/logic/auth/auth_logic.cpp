#include "auth_logic.h"
#include <iostream>
#include <cctype>
#include <algorithm>

AuthLogic::AuthLogic(AuthSender& authSender)
    : authSender(authSender), validationCallback_(nullptr) {}

void AuthLogic::setValidationCallback(ValidationCallback cb) {
    validationCallback_ = std::move(cb);
}

void AuthLogic::onSignupSender(
    const std::string& username,
    const std::string& password,
    const std::string& displayName,
    ValidationCallback callback) {
        std::cout << "[AuthLogic] onSignupSender called for username: " << username << std::endl;

        const auto hasSpace = [](const std::string& s) {
            return std::any_of(s.begin(), s.end(), [](unsigned char c) {
                return std::isspace(c) != 0;
            });
        };

        if (username.length() < 3 || username.length() > 40) {
            std::string msg = "[AuthLogic] Validation failed: username length must be 3-40 characters\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (hasSpace(username)) {
            std::string msg = "[AuthLogic] Validation failed: username cannot contain spaces\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (password.length() < 6 || password.length() > 50) {
            std::string msg = "[AuthLogic] Validation failed: password length must be 6-50 characters\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (hasSpace(password)) {
            std::string msg = "[AuthLogic] Validation failed: password cannot contain spaces\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        std::string msg = "[AuthLogic] Validation passed, sending signup\n";
        std::cout << msg;
        if (callback) callback(true, msg);
        if (validationCallback_) validationCallback_(true, msg);
        authSender.sendSignup(username, password, displayName);
}

void AuthLogic::onLoginSender(
    const std::string& username,
    const std::string& password,
    ValidationCallback callback) {
        std::cout << "[AuthLogic] onLoginSender called for username: " << username << std::endl;

        const auto hasSpace = [](const std::string& s) {
            return std::any_of(s.begin(), s.end(), [](unsigned char c) {
                return std::isspace(c) != 0;
            });
        };

        if (username.length() < 3 || username.length() > 40) {
            std::string msg = "[AuthLogic] Validation failed: username length must be 3-40 characters\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (hasSpace(username)) {
            std::string msg = "[AuthLogic] Validation failed: username cannot contain spaces\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (password.length() < 6 || password.length() > 50) {
            std::string msg = "[AuthLogic] Validation failed: password length must be 6-50 characters\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        if (hasSpace(password)) {
            std::string msg = "[AuthLogic] Validation failed: password cannot contain spaces\n";
            std::cout << msg;
            if (callback) callback(false, msg);
            if (validationCallback_) validationCallback_(false, msg);
            return;
        }

        std::string msg = "[AuthLogic] Validation passed, sending login\n";
        std::cout << msg;
        if (callback) callback(true, msg);
        if (validationCallback_) validationCallback_(true, msg);
        authSender.sendLogin(username, password);
}

void AuthLogic::getCurrentBalance(uint32_t userId) {
    std::cout << "[AuthLogic] getCurrentBalance called for userId=" << userId << "\n";
    authSender.requestBalance(userId);
}



