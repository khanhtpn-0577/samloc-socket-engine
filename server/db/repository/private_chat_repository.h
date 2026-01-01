#pragma once
#include <string>
#include <cstdint>
#include "../database.h"

struct FriendInfo {
    uint32_t userId;
    std::string username;};
    
class PrivateChatRepository {
public:
    explicit PrivateChatRepository(Database& db);

    bool save(
        uint32_t senderId,
        uint32_t receiverId,
        const std::string& message
    );

    std::vector<FriendInfo> getFriends(uint32_t userId);

private:
    Database& database;
};
