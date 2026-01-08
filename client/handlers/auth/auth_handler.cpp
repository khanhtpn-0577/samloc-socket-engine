#include "auth_handler.h"
#include <iostream>

AuthHandler::AuthHandler(AuthLogic& logic, ClientSession& session)
    : authLogic_(logic), session_(session) {}

std::string AuthHandler::parseField(const std::string& payload, const std::string& key) {
    // Simple JSON-like parsing: {"key":"value","key2":"value2"}
    std::string searchKey = "\"" + key + "\":\"";
    size_t keyPos = payload.find(searchKey);
    
    if (keyPos == std::string::npos) {
        return "";
    }
    
    size_t valueStart = keyPos + searchKey.length();
    size_t valueEnd = payload.find("\"", valueStart);
    
    if (valueEnd == std::string::npos) {
        return "";
    }
    
    return payload.substr(valueStart, valueEnd - valueStart);
}

uint32_t AuthHandler::parseUint32Field(const std::string& payload, const std::string& key) {
    // Parse number field: {"key":123}
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);
    
    if (keyPos == std::string::npos) {
        return 0;
    }
    
    size_t valueStart = keyPos + searchKey.length();
    size_t valueEnd = payload.find_first_of(",}", valueStart);
    
    if (valueEnd == std::string::npos) {
        return 0;
    }
    
    std::string valueStr = payload.substr(valueStart, valueEnd - valueStart);
    try {
        return std::stoul(valueStr);
    } catch (...) {
        return 0;
    }
}

bool AuthHandler::parseBoolField(const std::string& payload, const std::string& key) {
    // Parse bool field: {"key":true}
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);
    
    if (keyPos == std::string::npos) {
        return false;
    }
    
    size_t valueStart = keyPos + searchKey.length();
    return payload.substr(valueStart, 4) == "true";
}




void AuthHandler::onSignupResponse(const Message& message) {
    std::cout << "[AuthHandler] Received SIGNUP_RESPONSE\n";

    // 1. Parse đúng các field server gửi
    bool success = parseBoolField(message.payload, "success");
    std::string serverMsg = parseField(message.payload, "message");
    uint32_t userId = parseUint32Field(message.payload, "userId");

    // 2. Nếu signup thành công, cập nhật session tối thiểu
    if (success && userId > 0) {
        session_.setUserId(userId);
    }

    // 3. Gọi callback cho UI
    if (signupCallback_) {
        signupCallback_(
            success,
            userId,
            "",          // username: server không gửi
            "",          // displayName: server không gửi
            serverMsg
        );
    }
}


void AuthHandler::onLoginResponse(const Message& message) {
    std::cout << "[AuthHandler] Received LOGIN_RESPONSE\n";

    // 1. Parse đúng payload server gửi
    bool success = parseBoolField(message.payload, "success");
    std::string serverMsg = parseField(message.payload, "message");
    uint32_t userId = parseUint32Field(message.payload, "userId");
    std::string token = parseField(message.payload, "token");
    double balance = static_cast<double>(
        parseUint32Field(message.payload, "balance")
    );
    std::cout << "Login: " << (success ? "SUCCESS" : "FAILED")
              << ", UserId=" << userId
              << ", Token=" << token.substr(0, 8) << "..."
              << ", Balance=" << balance
              << ", Message=" << serverMsg << "\n";

    // 2. Nếu login thành công → update session
    if (success && userId > 0 && !token.empty()) {
        session_.setLoggedIn(true);
        session_.setUserId(userId);
        session_.setToken(token);
        session_.setBalance(balance);
        std::cout << "[AuthHandler] Login successful. UserId=" << userId
                  << ", Token=" << token.substr(0, 8) << "..., Balance=" << balance << "\n";
        session_.setState(ClientState::LOGGED_IN);
    }

    // 3. Gọi callback cho GUI
    if (loginCallback_) {
        loginCallback_(
            success,
            userId,
            token,
            serverMsg
        );
    }
}


void AuthHandler::onLogoutResponse(const Message& message) {
    std::cout << "[AuthHandler] Received LOGOUT_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    
    std::cout << "Logout: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        // Update session
        session_.setLoggedIn(false);
        session_.setUserId(0);
        session_.setToken("");
        session_.setState(ClientState::LOGGED_OUT);
        
        std::cout << "You are now logged out.\n";
    }
}

void AuthHandler::onSignupSender(const std::string& username, const std::string& password, const std::string& displayName) {
    std::cout << "[AuthHandler] Signup request: username=" << username << "\n";
    authLogic_.onSignupSender(username, password, displayName);
}

void AuthHandler::onLoginSender(const std::string& username, const std::string& password) {
    std::cout << "[AuthHandler] Login request: username=" << username << "\n";
    authLogic_.onLoginSender(username, password);
}

void AuthHandler::setSignupCallback(SignupCallback cb) {
    signupCallback_ = std::move(cb);
}

void AuthHandler::setLoginCallback(LoginCallback cb) {
    loginCallback_ = std::move(cb);
}

void AuthHandler::getCurrentBalance() {
    std::cout << "[AuthHandler] Requesting current balance\n";
    uint32_t userId = session_.userId();
    authLogic_.getCurrentBalance(userId);
}

void AuthHandler::onBalanceResponse(const Message& message) {
    std::cout << "[AuthHandler] Received REQUEST_BALANCE_RESPONSE\n";

    // Parse payload
    bool success = parseBoolField(message.payload, "success");
    std::string serverMsg = parseField(message.payload, "message");
    double balance = static_cast<double>(
        parseUint32Field(message.payload, "balance")
    );

    if (success) {
        session_.setBalance(balance);
        std::cout << "[AuthHandler] Balance updated: " << balance << "\n";
    } else {
        std::cout << "[AuthHandler] Failed to get balance: " << serverMsg << "\n";
    }
}