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

Message ChatHandler::handlePrivateChatHistoryRequest(
    const Message& incomingMsg
) {
    uint32_t userId = incomingMsg.header.senderId;

    // ===== Parse payload =====
    // payload: {"friendId":X}
    const std::string& payload = incomingMsg.payload;

    size_t pos = payload.find("\"friendId\":");
    if (pos == std::string::npos) {
        std::cerr << "[ChatHandler] Invalid history request payload\n";
    }

    size_t idStart = pos + 11;
    size_t idEnd   = payload.find("}", idStart);

    uint32_t friendId =
        static_cast<uint32_t>(
            std::stoul(payload.substr(idStart, idEnd - idStart))
        );

    std::string dbPath = "../../samloc.db";
    std::string responsePayload = "{\"messages\":[";

    try {
        Database db(dbPath);
        PrivateChatRepository repo(db);

        auto messages =
            repo.getPrivateChatHistory(userId, friendId);

        for (size_t i = 0; i < messages.size(); ++i) {
            const auto& m = messages[i];

            responsePayload += "{";
            responsePayload += "\"senderId\":" + std::to_string(m.senderId) + ",";
            responsePayload += "\"content\":\"";

            // escape quote
            for (char c : m.content) {
                if (c == '"') responsePayload += "\\\"";
                else responsePayload += c;
            }

            responsePayload += "\",";
            responsePayload += "\"sentAt\":\"" + m.sentAt + "\"}";
            if (i + 1 < messages.size()) responsePayload += ",";
        }
    } catch (...) {
        std::cerr << "[DB] Failed to load private chat history\n";
    }

    responsePayload += "]}";

    // ===== Build response message =====
    Message response;
    response.header.messageType =
        static_cast<uint16_t>(
            MessageType::PRIVATE_CHAT_HISTORY_RESPONSE
        );
    response.header.senderId = 0; // server
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = responsePayload.size();
    std::memset(response.header.token, 0, 32);

    response.payload = responsePayload;
    return response;
}


void ChatHandler::handleRoomChat(const Message& incomingMsg) {
    uint32_t senderId = incomingMsg.header.senderId;

    if (incomingMsg.payload.size() < sizeof(uint32_t)) {
        std::cerr << "[CHAT_ROOM] Invalid payload\n";
        return;
    }

    // payload layout:
    // [roomId (4 bytes)][message content...]
    uint32_t roomId;
    std::memcpy(&roomId, incomingMsg.payload.data(), sizeof(uint32_t));
    roomId = ntohl(roomId);

    std::string message =
        incomingMsg.payload.substr(sizeof(uint32_t));

    chatLogic.handleRoomChat(senderId, roomId, message);
}



