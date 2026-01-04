#include "rank_sender.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <sstream>
#ifdef _WIN32
    #include <winsock2.h> // Dành cho Windows (để VS Code không báo lỗi)
#else
    #include <arpa/inet.h> // Dành cho Linux/WSL
#endif
// Constructor
RankSender::RankSender(ClientSocket& socket, ClientSession& session)
    : socket(socket), session(session) {}

// Get current timestamp (milliseconds since epoch)
uint64_t RankSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// Create base message with header
Message RankSender::createMessage(MessageType type, const std::string& payload) {
    Message msg;
    
    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId = session.userId();
    msg.header.timestamp = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();
    
    // Set token
    setToken(msg.header, session.token());
    
    msg.payload = payload;

    std::cout << "[Client: rank_sender: Creating message] type=0x"
              << std::hex << msg.header.messageType
              << std::dec << ", senderId=" << msg.header.senderId
              << ", timestamp=" << msg.header.timestamp
              << ", payloadLength=" << msg.header.payloadLength
              << ", payload=" << msg.payload << "\n";
    
    return msg;
}


// Generic send message
bool RankSender::sendMessage(MessageType type, const std::string& payload) {
    Message msg = createMessage(type, payload);
    
    if (socket.sendMessage(msg)) {
        std::cout << "Message sent (type: " << static_cast<uint16_t>(type) << ")\n";
        return true;
    } else {
        std::cerr << "Failed to send message\n";
        return false;
    }
}
