#include "chat_logic.h"
#include "../../handler/session/session_manager.h"
#include <iostream>

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
