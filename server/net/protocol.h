#pragma once

#include <cstdint>
#include <string>

// Message Types (shared with client)
enum class MessageType : uint16_t {
    CHAT_DIRECT = 0x0001,
    CHAT_ROOM = 0x0002,
    JOIN_ROOM = 0x0010,
    LEAVE_ROOM = 0x0011,
    PLAY_CARDS = 0x0020,
    PASS_ROUND = 0x0021,
    DECLARE_SAM = 0x0022,
    SPIN_WHEEL = 0x0030,
    LOGIN = 0x0100,
    LOGOUT = 0x0101
};
