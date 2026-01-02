#include "auth_logic.h"
#include <iostream>
#include <cctype>
#include <algorithm>

AuthLogic::AuthLogic(AuthSender& authSender)
    : authSender(authSender) {}

void AuthLogic::onSignupSender(
    const std::string& username,
    const std::string& password,
    const std::string& displayName) {
        std::cout << "[AuthLogic] onSignupSender called for username: " << username << std::endl;
        authSender.sendSignup(username, password, displayName);
}

void AuthLogic::onLoginSender(
    const std::string& username,
    const std::string& password) {
        std::cout << "[AuthLogic] onLoginSender called for username: " << username << std::endl;
        authSender.sendLogin(username, password);
}



