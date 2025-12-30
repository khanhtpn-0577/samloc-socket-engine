#include "challenge_handler.h"
#include "../connection/connection_handler.h"
#include <chrono>
#include <iostream>
#include <sstream>

ChallengeHandler::ChallengeHandler(ChallengeLogic& challengeLogic)
    : challengeLogic(challengeLogic) {}

std::string ChallengeHandler::parseField(const std::string& payload, const std::string& key) {
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

uint32_t ChallengeHandler::parseUint32Field(const std::string& payload, const std::string& key) {
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

std::string ChallengeHandler::buildPayload(
    bool success,
    const std::string& message,
    uint32_t challengeId,
    uint32_t senderId,
    uint32_t receiverId
) {
    std::stringstream ss;
    ss << "{\"success\":" << (success ? "true" : "false")
       << ",\"message\":\"" << message << "\"";
    
    if (challengeId > 0) {
        ss << ",\"challengeId\":" << challengeId;
    }
    
    if (senderId > 0) {
        ss << ",\"senderId\":" << senderId;
    }
    
    if (receiverId > 0) {
        ss << ",\"receiverId\":" << receiverId;
    }
    
    ss << "}";
    return ss.str();
}

Message ChallengeHandler::createResponse(
    MessageType type,
    bool success,
    const std::string& message,
    uint32_t challengeId
) {
    Message response;
    response.header.messageType = static_cast<uint16_t>(type);
    response.header.senderId = 0; // Server
    response.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    response.payload = buildPayload(success, message, challengeId, 0, 0);
    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);
    
    return response;
}

void ChallengeHandler::notifyReceiver(uint32_t receiverId, uint32_t senderId, uint32_t challengeId) {
    // Find receiver's connection
    ConnectionHandler* receiverConn = SessionManager::instance().get(receiverId);
    
    if (!receiverConn) {
        std::cout << "[ChallengeHandler] Receiver " << receiverId << " not online\n";
        return;
    }
    
    // Build notification message
    Message notification;
    notification.header.messageType = static_cast<uint16_t>(MessageType::CHALLENGE_NOTIFICATION);
    notification.header.senderId = 0; // Server
    notification.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    notification.payload = buildPayload(true, "You have a new challenge", challengeId, senderId, receiverId);
    notification.header.payloadLength = notification.payload.size();
    std::memset(notification.header.token, 0, 32);
    
    // Send notification
    receiverConn->sendMessage(notification);
    
    std::cout << "[ChallengeHandler] Sent challenge notification to user " << receiverId << "\n";
}

Message ChallengeHandler::handleSendChallenge(const Message& incomingMsg) {
    std::cout << "[ChallengeHandler] Handling SEND_CHALLENGE\n";
    
    uint32_t senderId = incomingMsg.header.senderId;
    uint32_t receiverId = parseUint32Field(incomingMsg.payload, "receiverId");
    
    std::cout << "[ChallengeHandler] Send challenge: sender=" << senderId
              << " receiver=" << receiverId << "\n";
    
    // Call logic
    ChallengeResult result = challengeLogic.sendChallenge(senderId, receiverId);
    
    // If successful, notify receiver
    if (result.success) {
        notifyReceiver(receiverId, senderId, result.challengeId);
    }
    
    // Build response
    return createResponse(
        MessageType::SEND_CHALLENGE_RESPONSE,
        result.success,
        result.message,
        result.challengeId
    );
}

Message ChallengeHandler::handleAcceptChallenge(const Message& incomingMsg) {
    std::cout << "[ChallengeHandler] Handling ACCEPT_CHALLENGE\n";
    
    uint32_t receiverId = incomingMsg.header.senderId;
    uint32_t challengeId = parseUint32Field(incomingMsg.payload, "challengeId");
    
    std::cout << "[ChallengeHandler] Accept challenge: id=" << challengeId
              << " receiver=" << receiverId << "\n";
    
    // Call logic
    ChallengeResult result = challengeLogic.acceptChallenge(challengeId, receiverId);
    
    // TODO: If successful, notify sender and create room
    
    // Build response
    return createResponse(
        MessageType::ACCEPT_CHALLENGE_RESPONSE,
        result.success,
        result.message,
        challengeId
    );
}

Message ChallengeHandler::handleRejectChallenge(const Message& incomingMsg) {
    std::cout << "[ChallengeHandler] Handling REJECT_CHALLENGE\n";
    
    uint32_t receiverId = incomingMsg.header.senderId;
    uint32_t challengeId = parseUint32Field(incomingMsg.payload, "challengeId");
    
    std::cout << "[ChallengeHandler] Reject challenge: id=" << challengeId
              << " receiver=" << receiverId << "\n";
    
    // Call logic
    ChallengeResult result = challengeLogic.rejectChallenge(challengeId, receiverId);
    
    // TODO: If successful, notify sender
    
    // Build response
    return createResponse(
        MessageType::REJECT_CHALLENGE_RESPONSE,
        result.success,
        result.message,
        challengeId
    );
}

Message ChallengeHandler::handleCancelChallenge(const Message& incomingMsg) {
    std::cout << "[ChallengeHandler] Handling CANCEL_CHALLENGE\n";
    
    uint32_t senderId = incomingMsg.header.senderId;
    uint32_t challengeId = parseUint32Field(incomingMsg.payload, "challengeId");
    
    std::cout << "[ChallengeHandler] Cancel challenge: id=" << challengeId
              << " sender=" << senderId << "\n";
    
    // Call logic
    ChallengeResult result = challengeLogic.cancelChallenge(challengeId, senderId);
    
    // TODO: If successful, notify receiver
    
    // Build response
    return createResponse(
        MessageType::CANCEL_CHALLENGE_RESPONSE,
        result.success,
        result.message,
        challengeId
    );
}
