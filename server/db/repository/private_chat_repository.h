#pragma once
#include <string>
#include <cstdint>
#include "../database.h"

class PrivateChatRepository {
public:
    explicit PrivateChatRepository(Database& db);

    bool save(
        uint32_t senderId,
        uint32_t receiverId,
        const std::string& message
    );

private:
    Database& database;
};
