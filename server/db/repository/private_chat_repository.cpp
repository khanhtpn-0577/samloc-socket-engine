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
