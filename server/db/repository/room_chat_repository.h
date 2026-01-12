#pragma once
#include <vector>
#include <cstdint>
#include "../database.h"

class RoomChatRepository {
public:
    explicit RoomChatRepository(Database& db);

    std::vector<uint32_t> getRoomMemberIds(uint32_t roomId);

private:
    Database& database;
};
