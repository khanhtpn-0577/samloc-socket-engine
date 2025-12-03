#pragma once

#include <string>
#include <cstdint>
#include "../network/message_sender.h"

class ChatHandler {
public:
    ChatHandler(MessageSender& messageSender);
    
    // Send direct message
    bool sendDirectMessage(uint32_t receiverId, const std::string& message);
    
    // Send message to room
    bool sendRoomMessage(uint32_t roomId, const std::string& message);
    
    // Validate message
    bool isValidMessage(const std::string& message) const;

private:
    MessageSender& messageSender;
    
    static constexpr size_t MAX_MESSAGE_LENGTH = 1024;
};
