#pragma once
#include <string>
#include <cstdint>

class ChatLogic {
public:
    std::string handleDirectChat(
        uint32_t senderId,
        uint32_t receiverId,
        const std::string& message
    );
};
