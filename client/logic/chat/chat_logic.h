#pragma once

#include <string>
#include <cstdint>
#include "../../net/chat/message_sender.h"

/**
 * ChatLogic
 *  - Chịu trách nhiệm nghiệp vụ chat phía client
 *  - Validate dữ liệu người dùng
 *  - Quyết định có gửi message hay không
 */
class ChatLogic {
public:
    explicit ChatLogic(MessageSender& messageSender);

    // Gửi tin nhắn trực tiếp (1-1)
    bool sendDirectMessage(uint32_t receiverId, const std::string& message);

    // Gửi tin nhắn trong phòng
    bool sendRoomMessage(uint32_t roomId, const std::string& message);

private:
    bool isValidMessage(const std::string& message) const;

private:
    MessageSender& messageSender;

    static constexpr size_t MAX_MESSAGE_LENGTH = 1024;
};
