#include "challenge_handler.h"
#include <iostream>

ChallengeHandler::ChallengeHandler(ClientSession& session)
    : session(session) {}

std::string ChallengeHandler::parseField(const std::string& payload, const std::string& key) {
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

uint32_t ChallengeHandler::parseUint32Field(const std::string& payload, const std::string& key) {
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

bool ChallengeHandler::parseBoolField(const std::string& payload, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);
    
    if (keyPos == std::string::npos) {
        return false;
    }
    
    size_t valueStart = keyPos + searchKey.length();
    return payload.substr(valueStart, 4) == "true";
}

void ChallengeHandler::onChallengeNotification(const Message& message) {
    std::cout << "\n========================================\n";
    std::cout << "[CHALLENGE] You received a challenge!\n";
    
    uint32_t challengeId = parseUint32Field(message.payload, "challengeId");
    uint32_t senderId = parseUint32Field(message.payload, "senderId");
    
    std::cout << "Challenge ID: " << challengeId << "\n";
    std::cout << "From User ID: " << senderId << "\n";
    std::cout << "You have 30 seconds to accept or reject.\n";
    std::cout << "========================================\n\n";
}

void ChallengeHandler::onSendChallengeResponse(const Message& message) {
    std::cout << "[ChallengeHandler] Received SEND_CHALLENGE_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    uint32_t challengeId = parseUint32Field(message.payload, "challengeId");
    
    std::cout << "Send Challenge: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        std::cout << "Challenge ID: " << challengeId << "\n";
        std::cout << "Waiting for opponent to respond...\n";
    }
}

void ChallengeHandler::onAcceptChallengeResponse(const Message& message) {
    std::cout << "[ChallengeHandler] Received ACCEPT_CHALLENGE_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    
    std::cout << "Accept Challenge: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
    
    if (success) {
        std::cout << "Challenge accepted! Preparing game room...\n";
    }
}

void ChallengeHandler::onRejectChallengeResponse(const Message& message) {
    std::cout << "[ChallengeHandler] Received REJECT_CHALLENGE_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    
    std::cout << "Reject Challenge: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
}

void ChallengeHandler::onCancelChallengeResponse(const Message& message) {
    std::cout << "[ChallengeHandler] Received CANCEL_CHALLENGE_RESPONSE\n";
    
    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");
    
    std::cout << "Cancel Challenge: " << (success ? "SUCCESS" : "FAILED") << " - " << msg << "\n";
}

void ChallengeHandler::onChallengeExpired(const Message& message) {
    std::cout << "\n[CHALLENGE] Challenge has expired!\n";
    
    uint32_t challengeId = parseUint32Field(message.payload, "challengeId");
    std::cout << "Challenge ID: " << challengeId << "\n\n";
}
