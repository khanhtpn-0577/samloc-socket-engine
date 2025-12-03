#include "chat_handler.h"
#include <iostream>

// Constructor
ChatHandler::ChatHandler(MessageSender& messageSender)
    : messageSender(messageSender) {
}

// Validate message
bool ChatHandler::isValidMessage(const std::string& message) const {
    if (message.empty()) {
        std::cerr << "Message cannot be empty\n";
        return false;
    }
    
    if (message.size() > MAX_MESSAGE_LENGTH) {
        std::cerr << "Message too long (max " << MAX_MESSAGE_LENGTH << " characters)\n";
        return false;
    }
    
    return true;
}

// Send direct message
bool ChatHandler::sendDirectMessage(uint32_t receiverId, const std::string& message) {
    if (!isValidMessage(message)) {
        return false;
    }
    
    if (receiverId == 0) {
        std::cerr << "Invalid receiver ID\n";
        return false;
    }
    
    return messageSender.sendDirectMessage(receiverId, message);
}

// Send message to room
bool ChatHandler::sendRoomMessage(uint32_t roomId, const std::string& message) {
    if (!isValidMessage(message)) {
        return false;
    }
    
    if (roomId == 0) {
        std::cerr << "Invalid room ID\n";
        return false;
    }
    
    return messageSender.sendRoomMessage(roomId, message);
}
