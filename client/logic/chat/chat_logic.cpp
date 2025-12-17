#include "chat_logic.h"
#include <iostream>

ChatLogic::ChatLogic(MessageSender& messageSender)
    : messageSender(messageSender) {
}

bool ChatLogic::isValidMessage(const std::string& message) const {
    if (message.empty()) {
        std::cerr << "Message cannot be empty\n";
        return false;
    }

    if (message.size() > MAX_MESSAGE_LENGTH) {
        std::cerr << "Message too long (max "
                  << MAX_MESSAGE_LENGTH << " characters)\n";
        return false;
    }

    return true;
}

bool ChatLogic::sendDirectMessage(
    uint32_t receiverId,
    const std::string& message
) {
    if (receiverId == 0) {
        std::cerr << "Invalid receiver ID\n";
        return false;
    }

    if (!isValidMessage(message)) {
        return false;
    }

    return messageSender.sendDirectMessage(receiverId, message);
}

bool ChatLogic::sendRoomMessage(
    uint32_t roomId,
    const std::string& message
) {
    if (roomId == 0) {
        std::cerr << "Invalid room ID\n";
        return false;
    }

    if (!isValidMessage(message)) {
        return false;
    }

    return messageSender.sendRoomMessage(roomId, message);
}
