#include "chat_logic.h"
#include "../../handler/session/session_manager.h"
#include <iostream>
#include "../../db/repository/room_chat_repository.h"
#include "../../db/database.h"
#include <arpa/inet.h>


ChatLogic::ChatLogic() = default;

std::string ChatLogic::handleDirectChat(
    uint32_t senderId,
    uint32_t receiverId,
    const std::string& message,
    const Message& incomingMsg
) {
    // ===== validate =====
    if (senderId == 0 || receiverId == 0) {
        return "Invalid sender or receiver";
    }

    if (message.empty()) {
        return "Empty message";
    }

    // ===== business =====
    std::cout << "[CHAT_DIRECT] "
              << senderId << " -> "
              << receiverId << " : "
              << message << std::endl;

    // ===== forward =====
    ConnectionHandler* receiverConn =
        SessionManager::instance().get(receiverId);

    if (receiverConn) {
        messageSender.sendDirectMessage(
            receiverConn,
            senderId,
            incomingMsg
        );
    }

    return "Message delivered to server";
}

void ChatLogic::handleRoomChat(
    uint32_t senderId,
    uint32_t roomId,
    const std::string& message
) {
    // ===== validate =====
    if (senderId == 0 || roomId == 0) {
        std::cerr << "[CHAT_ROOM] Invalid sender or room\n";
        return;
    }

    if (message.empty()) {
        std::cerr << "[CHAT_ROOM] Empty message\n";
        return;
    }

    std::cout << "[CHAT_ROOM] sender=" << senderId
              << " room=" << roomId
              << " msg=" << message << "\n";

    // ===== get members in room =====
    std::string dbPath = "../../samloc.db";
    Database db(dbPath);
    RoomChatRepository repo(db);

    auto memberIds = repo.getRoomMemberIds(roomId);

    // ===== build broadcast message =====
    // payload: senderId (4 bytes) + message
    std::string payload;
    uint32_t netSenderId = htonl(senderId);
    payload.append(reinterpret_cast<const char*>(&netSenderId), sizeof(uint32_t));
    payload.append(message);

    Message outMsg;
    outMsg.header.messageType =
        static_cast<uint16_t>(MessageType::S_ROOM_CHAT);
    outMsg.header.senderId = 0; // server
    outMsg.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    outMsg.payload = payload;

    // ===== broadcast =====
    for (uint32_t uid : memberIds) {
        ConnectionHandler* conn =
            SessionManager::instance().get(uid);

        if (conn) {
            conn->sendMessage(outMsg);
        }
    }
}
