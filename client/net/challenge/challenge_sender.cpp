#include "challenge_sender.h"
#include <iostream>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

ChallengeSender::ChallengeSender(ClientSocket& socket, ClientSession& session)
    : socket_(socket), session_(session) {}

// Get current timestamp (milliseconds since epoch)
uint64_t ChallengeSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Create base message with header
Message ChallengeSender::createMessage(MessageType type, const std::string& payload) {
    Message msg;

    msg.header.messageType   = static_cast<uint16_t>(type);
    msg.header.senderId      = session_.userId();
    msg.header.timestamp     = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();

    // Attach token
    setToken(msg.header, session_.token());

    msg.payload = payload;

    std::cout << "[Client: challenge_sender] Create message "
              << "type=0x" << std::hex << msg.header.messageType
              << std::dec << ", senderId=" << msg.header.senderId
              << ", timestamp=" << msg.header.timestamp
              << ", payloadLength=" << msg.header.payloadLength
              << ", payload=" << msg.payload << "\n";

    return msg;
}

// Generic send message
bool ChallengeSender::sendMessage(MessageType type, const std::string& payload) {
    Message msg = createMessage(type, payload);

    if (socket_.sendMessage(msg)) {
        std::cout << "[ChallengeSender] Message sent (type="
                  << static_cast<uint16_t>(type) << ")\n";
        return true;
    } else {
        std::cerr << "[ChallengeSender] Failed to send message\n";
        return false;
    }
}

void ChallengeSender::updateIdentity(uint32_t newUserId, const std::string& newToken) {
    session_.setUserId(newUserId);
    session_.setToken(newToken);
}
