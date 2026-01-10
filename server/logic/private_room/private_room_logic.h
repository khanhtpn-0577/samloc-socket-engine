#pragma once
#include <string>
#include <cstdint>
#include "../../db/database.h"

class PrivateRoomLogic {
public:
    explicit PrivateRoomLogic(Database& db);

    std::string createPrivateRoom(
        uint32_t createdBy,
        const std::string& roomName,
        const std::string& roomType,
        int betAmount
    );

private:
    Database& database_;
};
