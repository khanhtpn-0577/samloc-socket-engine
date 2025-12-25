#pragma once

#include "../../net/protocol.h"
#include "../../handler/connection/connection_handler.h"
#include <cstdint>
#include <string>

class MessageSender {
public:
    MessageSender() = default;

    // gửi message direct tới receiver
    bool sendDirectMessage(
        ConnectionHandler* receiverConn,
        uint32_t senderId,
        const Message& incomingMsg
    );
};
