#pragma once

#include "../../db/database.h"
#include <string>
#include <cstdint>

struct PrivateRoomInsertResult {
    uint32_t roomId;
    uint32_t roomCode;
};

class RoomPrivateRepository {
public:
    explicit RoomPrivateRepository(Database& db);

    // Insert private room, return roomId + roomCode
    PrivateRoomInsertResult createPrivateRoom(
        const std::string& roomName,
        const std::string& roomType,
        int betAmount,
        uint32_t createdBy
    );

private:
    Database& database_;
};
