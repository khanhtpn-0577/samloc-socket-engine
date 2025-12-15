#include "chat_logic.h"
#include <iostream>

std::string ChatLogic::handleDirectChat(
    uint32_t senderId,
    uint32_t receiverId,
    const std::string& message
) {
    std::cout << "[CHAT_DIRECT] "
              << senderId << " -> " << receiverId
              << " | " << message << std::endl;

    return "Server received your message";
}
