#pragma once

#include <string>
#include <cstdint>

#include "../../logic/chat/chat_logic.h"
#include "../session/client_session.h"
#include "../../net/protocol.h"


/**
 * ChatHandler
 *  - Entry point cho các use case chat
 *  - Được GUI gọi
 *  - Quan ly FSM
 */

class ChatLogic;
class ClientSession;

class ChatHandler {
    public:
        ChatHandler(ChatLogic& logic, ClientSession& session);

        void onSendPrivateChat(uint32_t receiverId, const std::string& message);


        void onServerACK(const Message& ackMsg);

        void onServerDeliverMessage(const Message& msg);

    private:
        ChatLogic& chatLogic_;
        ClientSession& session_;
};
