#pragma once

#include "../../net/protocol.h"
#include "../../db/database.h"
#include <string>
#include <vector>

class ConnectionHandler {
public:
    ConnectionHandler(int clientFd, Database& db);
    ~ConnectionHandler() = default;

    // gọi khi poll() báo fd readable
    bool onReadable();

    void closeConnection();
    int getFd() const;

    bool sendMessage(const Message& msg);

private:
    bool sendAll(const char* data, size_t size);
    bool recvAll(void* buffer, size_t size);
    

private:
    int clientFd;
    uint32_t boundUserId;
    Database& db;

    //buffer luu du lieu doc duoc tu tcp stream
    std::vector<char> inputBuffer;
    void processIncomingMessage(const Message& msg);
};
