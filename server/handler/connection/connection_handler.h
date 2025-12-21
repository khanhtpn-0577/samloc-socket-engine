#pragma once

#include <string>
#include <cstdint>
#include "../../net/protocol.h"

class ChatLogic;

class ConnectionHandler {
public:
    explicit ConnectionHandler(int clientFd);

    void handle();
    bool sendMessage(const Message& msg);

private:
    bool recvAll(void* buffer, size_t size);
    bool sendAll(const void* buffer, size_t size);

private:
    int clientFd;
};
