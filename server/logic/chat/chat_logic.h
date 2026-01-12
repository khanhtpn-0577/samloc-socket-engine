#pragma once

#include <string>
#include <cstdint>
#include "../../net/chat/message_sender.h"

class ChatLogic {
public:
    ChatLogic();

    std::string handleDirectChat(
        uint32_t senderId,
        uint32_t receiverId,
        const std::string& message,
        const Message& incomingMsg
    );

    void handleRoomChat(
        uint32_t senderId,
        uint32_t roomId,
        const std::string& message
    );

private:
    MessageSender messageSender;
};
