#pragma once

#include "../../net/protocol.h"
#include "../../logic/chat/chat_logic.h"

class ChatHandler {
public:
    explicit ChatHandler(ChatLogic& chatLogic);

    // entry point cho CHAT_DIRECT
    Message handleChatDirect(const Message& incomingMsg);

private:
    ChatLogic& chatLogic;
};
