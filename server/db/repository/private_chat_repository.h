#pragma once
#include <string>
#include <cstdint>
#include "../database.h"

struct FriendInfo {
    uint32_t userId;
    std::string username;};

struct ChatMessage {
    uint32_t senderId;
    uint32_t receiverId;
    std::string content;
    std::string sentAt;
};


class PrivateChatRepository {
public:
    explicit PrivateChatRepository(Database& db);

    bool save(
        uint32_t senderId,
        uint32_t receiverId,
        const std::string& message
    );

    std::vector<FriendInfo> getFriends(uint32_t userId);

    std::vector<ChatMessage> getPrivateChatHistory(uint32_t userId, uint32_t friendId);


private:
    Database& database;
};
