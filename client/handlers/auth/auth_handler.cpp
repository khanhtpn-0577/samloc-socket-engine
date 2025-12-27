#include "auth_handler.h"
#include <iostream>

AuthHandler::AuthHandler(ClientSession& session)
    : session(session) {}

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
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    uint32_t userId = parseUint32Field(message.payload, "userId");
    
    std::cout << "Signup: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        std::cout << "User ID: " << userId << "\n";
        std::cout << "You can now login with your credentials.\n";
    }
}

void AuthHandler::onLoginResponse(const Message& message) {
    std::cout << "[AuthHandler] Received LOGIN_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    uint32_t userId = parseUint32Field(message.payload, "userId");
    std::string token = parseField(message.payload, "token");
    
    std::cout << "Login: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        // Update session
        session.setLoggedIn(true);
        session.setUserId(userId);
        session.setToken(token);
        session.setState(ClientState::LOGGED_IN);
        
        std::cout << "User ID: " << userId << "\n";
        std::cout << "Session token: " << token.substr(0, 16) << "...\n";
        std::cout << "You are now logged in!\n";
    }
}

void AuthHandler::onLogoutResponse(const Message& message) {
    std::cout << "[AuthHandler] Received LOGOUT_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    
    std::cout << "Logout: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        // Update session
        session.setLoggedIn(false);
        session.setUserId(0);
        session.setToken("");
        session.setState(ClientState::LOGGED_OUT);
        
        std::cout << "You are now logged out.\n";
    }
}
