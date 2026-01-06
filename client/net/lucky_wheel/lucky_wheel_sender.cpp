#include "lucky_wheel_sender.h"
#include <iostream>
#include <chrono>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

// ================= CONSTRUCTOR =================

LuckyWheelSender::LuckyWheelSender(ClientSocket& socket, ClientSession& session)
    : socket_(socket),
      session_(session) {}

// ================= TIME =================

uint64_t LuckyWheelSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// ================= MESSAGE =================

Message LuckyWheelSender::createMessage(
    MessageType type,
    const std::string& payload
) {
    Message msg;

    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId    = session_.userId();
    msg.header.timestamp   = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();

    // Set token
    setToken(msg.header, session_.token());

    msg.payload = payload;

    std::cout << "[Client: lucky_wheel_sender: Creating message] type=0x"
              << std::hex << msg.header.messageType
              << std::dec
              << ", senderId=" << msg.header.senderId
              << ", timestamp=" << msg.header.timestamp
              << ", payloadLength=" << msg.header.payloadLength
              << ", payload=" << msg.payload << "\n";

    return msg;
}

// ================= SEND =================

bool LuckyWheelSender::sendMessage(
    MessageType type,
    const std::string& payload
) {
    Message msg = createMessage(type, payload);

    if (socket_.sendMessage(msg)) {
        std::cout << "[LuckyWheelSender] Message sent (type="
                  << static_cast<uint16_t>(type) << ")\n";
        return true;
    }

    std::cerr << "[LuckyWheelSender] Failed to send message\n";
    return false;
}
