#include "chat_handler.h"
#include <cstring>
#include <arpa/inet.h>
#include <chrono>
#include "../session/session_manager.h"
#include "../../db/repository/private_chat_repository.h"
#include "../../db/database.h"
#include <iostream>



ChatHandler::ChatHandler(ChatLogic& chatLogic)
    : chatLogic(chatLogic) {}

Message ChatHandler::handleChatDirect(const Message& incomingMsg) {
    std::string dbPath = "../../samloc.db";
    uint32_t receiverId;
    std::memcpy(
        &receiverId,
        incomingMsg.payload.data(),
        sizeof(uint32_t)
    );
    receiverId = ntohl(receiverId);

    std::string message =
        incomingMsg.payload.substr(sizeof(uint32_t));

    //===== logic =====
    std::string ackText =
        chatLogic.handleDirectChat(
            incomingMsg.header.senderId,
            receiverId,
            message,
            incomingMsg
        );

    // ===== SAVE TO DATABASE =====
    try {
        Database db(dbPath); // hoặc lấy từ singleton
        PrivateChatRepository repo(db);

        repo.save(
            incomingMsg.header.senderId,
            receiverId,
            message
        );
    } catch (...) {
        std::cerr << "[DB] Exception while saving private chat\n";
    }
    // std::string ackText = "ACK from server";

    // ===== build ACK =====
    Message ackMsg;
    ackMsg.header.messageType =
        static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK);
    ackMsg.header.senderId = 0;
    ackMsg.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    ackMsg.header.payloadLength = ackText.size();
    std::memset(ackMsg.header.token, 0, 32);

    ackMsg.payload = ackText;
    return ackMsg;
}

Message ChatHandler::handleFriendListRequest(
    const Message& incomingMsg
) {
    uint32_t userId = incomingMsg.header.senderId;
    std::cout << "[Server] Handling FRIEND_LIST_REQUEST for userId="
              << userId << "\n";
    std::string dbPath = "../../samloc.db";

    std::string payload = R"({"friends":[)";

    try {
        Database db(dbPath);
        PrivateChatRepository repo(db);

        auto friends = repo.getFriends(userId);

        for (size_t i = 0; i < friends.size(); ++i) {
            payload += "{";
            payload += "\"id\":" + std::to_string(friends[i].userId) + ",";
            payload += "\"name\":\"" + friends[i].username + "\"}";
            if (i + 1 < friends.size()) payload += ",";
        }
    } catch (...) {
        std::cerr << "[DB] Failed to load friend list\n";
    }

    payload += "]}";

    Message response;
    response.header.messageType =
        static_cast<uint16_t>(MessageType::FRIEND_LIST_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = payload.size();
    std::memset(response.header.token, 0, 32);
    response.payload = payload;

    return response;
}


