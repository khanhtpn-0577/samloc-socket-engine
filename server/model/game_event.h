#pragma once

#include <string>
#include <vector>

#include "../net/protocol.h" 

struct GameEvent {
    MessageType type;
    std::string payload;
    std::vector<int> targetPlayerIds;
    int roomId = 0; // roomId để broadcast trong phòng
};