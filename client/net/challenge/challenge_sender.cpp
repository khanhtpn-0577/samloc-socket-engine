#include "challenge_sender.h"
#include <sstream>
#include <chrono>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

ChallengeSender::ChallengeSender(ClientSocket& socket, uint32_t userId, const std::string& token)
    : socket(socket), userId(userId), token(token) {}

uint64_t ChallengeSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

Message ChallengeSender::createMessage(MessageType type, const std::string& payload) {
    Message msg;
    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId = userId;
    msg.header.timestamp = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();

    setToken(msg.header, token);
    msg.payload = payload;
    return msg;
}

bool ChallengeSender::sendMessage(MessageType type, const std::string& payload) {
    Message msg = createMessage(type, payload);
    if (socket.sendMessage(msg)) {
        std::cout << "Message sent (type: " << static_cast<uint16_t>(type) << ")\n";
        return true;
    }
    std::cerr << "Failed to send message\n";
    return false;
}

bool ChallengeSender::sendChallenge(uint32_t receiverId) {
    std::stringstream payload;
    payload << "{\"receiverId\":" << receiverId << "}";
    if (sendMessage(MessageType::SEND_CHALLENGE, payload.str())) {
        std::cout << "Challenge sent to user " << receiverId << "\n";
        return true;
    }
    return false;
}

bool ChallengeSender::acceptChallenge(uint32_t challengeId) {
    std::stringstream payload;
    payload << "{\"challengeId\":" << challengeId << "}";
    if (sendMessage(MessageType::ACCEPT_CHALLENGE, payload.str())) {
        std::cout << "Challenge accepted: " << challengeId << "\n";
        return true;
    }
    return false;
}

bool ChallengeSender::rejectChallenge(uint32_t challengeId) {
    std::stringstream payload;
    payload << "{\"challengeId\":" << challengeId << "}";
    if (sendMessage(MessageType::REJECT_CHALLENGE, payload.str())) {
        std::cout << "Challenge rejected: " << challengeId << "\n";
        return true;
    }
    return false;
}

bool ChallengeSender::cancelChallenge(uint32_t challengeId) {
    std::stringstream payload;
    payload << "{\"challengeId\":" << challengeId << "}";
    if (sendMessage(MessageType::CANCEL_CHALLENGE, payload.str())) {
        std::cout << "Challenge cancelled: " << challengeId << "\n";
        return true;
    }
    return false;
}

void ChallengeSender::updateIdentity(uint32_t newUserId, const std::string& newToken) {
    userId = newUserId;
    token = newToken;
}
