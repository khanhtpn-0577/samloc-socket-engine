#include "message_sender.h"
#include <cstring>

bool MessageSender::sendDirectMessage(
    ConnectionHandler* receiverConn,
    uint32_t senderId,
    const Message& incomingMsg
) {
    if (!receiverConn) return false;

    Message forwardMsg;
    forwardMsg.header.messageType =
        static_cast<uint16_t>(MessageType::CHAT_DIRECT);
    forwardMsg.header.senderId = senderId;
    forwardMsg.header.timestamp = incomingMsg.header.timestamp;
    forwardMsg.header.payloadLength = incomingMsg.payload.size();
    std::memcpy(forwardMsg.header.token, incomingMsg.header.token, 32);
    forwardMsg.payload = incomingMsg.payload;

    return receiverConn->sendMessage(forwardMsg);
}

