#pragma once

#include <string>
#include <cstdint>
#include "../../net/chat/message_sender.h"

/**
 * ChatLogic
 *  - Nghiệp vụ chat phía client
 *  - Validate dữ liệu
 *  - Gọi net layer
 */
class ChatLogic {
public:
    explicit ChatLogic(MessageSender& messageSender);

    bool sendDirectMessage(uint32_t receiverId,
                           const std::string& message);

    bool sendRoomMessage(uint32_t roomId,
                         const std::string& message);

    void requestFriendList(uint32_t userId);

private:
    bool isValidMessage(const std::string& message) const;

private:
    MessageSender& messageSender;
    static constexpr size_t MAX_MESSAGE_LENGTH = 1024;
};
