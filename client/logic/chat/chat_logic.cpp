#include "chat_logic.h"
#include <iostream>

ChatLogic::ChatLogic(MessageSender& messageSender)
    : messageSender(messageSender) {}

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

bool ChatLogic::sendDirectMessage(uint32_t receiverId,
                                  const std::string& message) {
    if (receiverId == 0) {
        std::cerr << "Invalid receiver ID\n";
        return false;
    }

    if (!isValidMessage(message)) {
        return false;
    }

    return messageSender.sendDirectMessage(receiverId, message);
}

bool ChatLogic::sendRoomMessage(uint32_t roomId,
                                const std::string& message) {
    if (roomId == 0) {
        std::cerr << "Invalid room ID\n";
        return false;
    }

    if (!isValidMessage(message)) {
        return false;
    }

    return messageSender.sendRoomMessage(roomId, message);
}

void ChatLogic::requestFriendList(uint32_t userId){
    if(userId == 0){
        std::cerr << "Invalid user ID for friend list request\n";
        return;
    }

    // Construct payload (e.g., JSON format)
    std::string payload = "{\"userId\":" + std::to_string(userId) + "}";

    if(!messageSender.sendMessage(MessageType::FRIEND_LIST_REQUEST, payload)){
        std::cerr << "[ChatLogic] Failed to send friend list request\n";
    } else {
        std::cout << "[ChatLogic] Friend list request sent for userId="
                  << userId << "\n";
    }
}

void ChatLogic::requestPrivateChatHistory(uint32_t friendId){
    if(friendId == 0){
        std::cerr << "Invalid friend ID for private chat history request\n";
        return;
    }

    // Construct payload (e.g., JSON format)
    std::string payload = "{\"friendId\":" + std::to_string(friendId) + "}";

    if(!messageSender.sendMessage(MessageType::PRIVATE_CHAT_HISTORY_REQUEST, payload)){
        std::cerr << "[ChatLogic] Failed to send private chat history request\n";
    } else {
        std::cout << "[ChatLogic] Private chat history request sent for friendId="
                  << friendId << "\n";
    }
}

std::string ChatLogic::normalizeChatContent(
    const std::string& raw
) const {
    if (raw.size() <= 4) {
        return "";
    }

    // Bỏ 4 byte đầu
    return raw.substr(4);
}
