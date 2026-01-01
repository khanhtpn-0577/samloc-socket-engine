#include "private_chat_repository.h"
#include <sstream>
#include <iostream>

PrivateChatRepository::PrivateChatRepository(Database& db)
    : database(db) {}

bool PrivateChatRepository::save(
    uint32_t senderId,
    uint32_t receiverId,
    const std::string& message
) {
    std::stringstream sql;
    sql <<
        "INSERT INTO private_chats (sender_id, receiver_id, message) VALUES ("
        << senderId << ", "
        << receiverId << ", '";

    // escape single quote (basic)
    for (char c : message) {
        if (c == '\'') sql << "''";
        else sql << c;
    }

    sql << "');";

    if (!database.execute(sql.str())) {
        std::cerr << "[DB] Failed to save private chat\n";
        return false;
    }

    return true;
}

std::vector<FriendInfo>
PrivateChatRepository::getFriends(uint32_t userId) {
    std::vector<FriendInfo> result;

    std::string sql =
        "SELECT p.player_id, p.username "
        "FROM friends f "
        "JOIN players p ON p.player_id = f.friend_id "
        "WHERE f.player_id = ?;";

    QueryResult rows =
        database.queryPrepared(sql, { std::to_string(userId) });

    for (const auto& row : rows) {
        FriendInfo f;
        f.userId = std::stoul(row.at("player_id"));
        f.username = row.at("username");
        result.push_back(f);
    }

    return result;
}


std::vector<ChatMessage>
PrivateChatRepository::getPrivateChatHistory(
    uint32_t userId,
    uint32_t friendId
) {
    std::vector<ChatMessage> result;

    std::string sql =
        "SELECT sender_id, receiver_id, message, sent_at "
        "FROM private_chats "
        "WHERE "
        "(sender_id = ? AND receiver_id = ?) "
        "OR "
        "(sender_id = ? AND receiver_id = ?) "
        "ORDER BY sent_at ASC;";

    QueryResult rows = database.queryPrepared(
        sql,
        {
            std::to_string(userId),
            std::to_string(friendId),
            std::to_string(friendId),
            std::to_string(userId)
        }
    );

    for (const auto& row : rows) {
        ChatMessage msg;
        msg.senderId   = std::stoul(row.at("sender_id"));
        msg.receiverId = std::stoul(row.at("receiver_id"));
        msg.content    = row.at("message");
        msg.sentAt     = row.at("sent_at");
        result.push_back(std::move(msg));
    }

    return result;
}

