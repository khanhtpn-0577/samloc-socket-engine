#pragma once

#include "../../net/protocol.h"

class ChatHandler;

class ClientConnectionHandler {
public:
    ClientConnectionHandler(ChatHandler& chatHandler);

    // gọi khi client nhận được message từ server
    void handleMessage(const Message& message);

private:
    ChatHandler& chatHandler_;
};
