#pragma once

#include <string>
#include <cstdint>

class ChatLogic;

class ConnectionHandler {
public:
    explicit ConnectionHandler(int clientFd);

    void handle();

private:
    bool recvAll(void* buffer, size_t size);
    bool sendAll(const void* buffer, size_t size);

private:
    int clientFd;
};
