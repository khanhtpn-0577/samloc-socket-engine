#include "chat_handler.h"
#include <cstring>
#include <arpa/inet.h>
#include <chrono>
#include "../session/session_manager.h"

ChatHandler::ChatHandler(ChatLogic& chatLogic)
    : chatLogic(chatLogic) {}

Message ChatHandler::handleChatDirect(const Message& incomingMsg) {
    uint32_t receiverId;
    std::memcpy(
        &receiverId,
        incomingMsg.payload.data(),
        sizeof(uint32_t)
    );
    receiverId = ntohl(receiverId);

    std::string message =
        incomingMsg.payload.substr(sizeof(uint32_t));

    //===== chỉ gọi logic =====
    std::string ackText =
        chatLogic.handleDirectChat(
            incomingMsg.header.senderId,
            receiverId,
            message,
            incomingMsg
        );
    // std::string ackText = "ACK from server";

    // ===== build ACK =====
    Message ackMsg;
    ackMsg.header.messageType =
        static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK);
    ackMsg.header.senderId = 0;
    ackMsg.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    ackMsg.header.payloadLength = ackText.size();
    std::memset(ackMsg.header.token, 0, 32);

    ackMsg.payload = ackText;
    return ackMsg;
}

