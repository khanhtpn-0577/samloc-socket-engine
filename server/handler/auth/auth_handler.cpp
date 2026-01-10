#include "auth_handler.h"
#include <chrono>
#include <iostream>
#include <sstream>

AuthHandler::AuthHandler(AuthLogic& authLogic)
    : authLogic(authLogic) {}

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
    // Parse numeric field: {"key":123}
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
    return std::stoul(valueStr);
}

std::string AuthHandler::buildPayload(
    bool success,
    const std::string& message,
    uint32_t userId,
    const std::string& token,
    int64_t balance
) {
    std::stringstream ss;
    ss << "{\"success\":" << (success ? "true" : "false")
       << ",\"message\":\"" << message << "\"";

    if (success) {
        ss << ",\"userId\":" << userId;
        ss << ",\"balance\":" << balance;

        if (!token.empty()) {
            ss << ",\"token\":\"" << token << "\"";
        }
    }

    ss << "}";
    return ss.str();
}


Message AuthHandler::createResponse(
    MessageType type,
    bool success,
    const std::string& message,
    uint32_t userId,
    const std::string& token,
    int64_t balance
) {
    Message response;
    response.header.messageType = static_cast<uint16_t>(type);
    response.header.senderId = 0; // Server
    response.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    response.payload = buildPayload(success, message, userId, token, balance);
    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);
    
    return response;
}

Message AuthHandler::handleSignup(const Message& incomingMsg) {
    std::cout << "[AuthHandler] Handling SIGNUP\n";
    
    // Parse payload: {"u":"username","p":"password","d":"displayName"}
    std::string username = parseField(incomingMsg.payload, "u");
    std::string password = parseField(incomingMsg.payload, "p");
    std::string displayName = parseField(incomingMsg.payload, "d");
    
    std::cout << "[AuthHandler] Signup request: username=" << username << "\n";
    
    // Call logic
    SignupResult result = authLogic.signup(username, password, displayName);
    
    // Build response
    return createResponse(
        MessageType::SIGNUP_RESPONSE,
        result.success,
        result.message,
        result.userId,
        ""
    );
}

Message AuthHandler::handleLogin(const Message& incomingMsg) {
    std::cout << "[AuthHandler] Handling LOGIN\n";
    
    // Parse payload: {"u":"username","p":"password"}
    std::string username = parseField(incomingMsg.payload, "u");
    std::string password = parseField(incomingMsg.payload, "p");
    
    std::cout << "[AuthHandler] Login request: username=" << username << "\n";
    
    // Call logic
    LoginResult result = authLogic.login(username, password);
    
    // Build response
    return createResponse(
        MessageType::LOGIN_RESPONSE,
        result.success,
        result.message,
        result.userId,
        result.token,
        result.balance
    );
}

Message AuthHandler::handleLogout(const Message& incomingMsg) {
    std::cout << "[AuthHandler] Handling LOGOUT\n";
    
    // Parse payload: {"token":"..."}
    std::string token = parseField(incomingMsg.payload, "token");
    
    if (token.empty()) {
        // Try to get from header
        token = getToken(incomingMsg.header);
        // Trim trailing zeros
        size_t endPos = token.find('\0');
        if (endPos != std::string::npos) {
            token = token.substr(0, endPos);
        }
    }
    
    std::cout << "[AuthHandler] Logout request: token=" << token.substr(0, 8) << "...\n";
    
    // Call logic
    bool success = authLogic.logout(token);
    
    // Build response
    return createResponse(
        MessageType::LOGOUT_RESPONSE,
        success,
        success ? "Logout successful" : "Logout failed",
        0,
        ""
    );
}

Message AuthHandler::handleRequestBalance(const Message& incomingMsg) {
    std::cout << "[AuthHandler] Handling REQUEST_BALANCE\n";
    
    uint32_t userId = parseUint32Field(incomingMsg.payload, "userId");
    std::cout << "[AuthHandler] Request balance for userId=" << userId << "\n";

    UserInfo userInfo = authLogic.getUserInfo(userId);
    LoginResult result;
    result.success = (userInfo.userId != 0);

    result.message = result.success ? "User info retrieved successfully" : "Failed to retrieve user info";
    result.balance = userInfo.balance;

    return createResponse(
        MessageType::REQUEST_BALANCE_RESPONSE,
        result.success,
        result.message,
        userId,
        "",
        result.balance
    );
}