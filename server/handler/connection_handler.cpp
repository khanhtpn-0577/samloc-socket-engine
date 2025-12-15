#include "connection_handler.h"
#include "../net/protocol.h"
#include "../logic/chat/chat_logic.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

#pragma pack(push, 1)
struct MessageHeader {
    uint16_t messageType;
    uint32_t senderId;
    uint64_t timestamp;
    uint8_t  token[32];
    uint32_t payloadLength;
};
#pragma pack(pop)

ConnectionHandler::ConnectionHandler(int clientFd)
    : clientFd(clientFd) {}

bool ConnectionHandler::recvAll(void* buffer, size_t size) {
    size_t received = 0;
    char* buf = static_cast<char*>(buffer);

    while (received < size) {
        ssize_t r = recv(clientFd, buf + received, size - received, 0);
        if (r <= 0) return false;
        received += r;
    }
    return true;
}

bool ConnectionHandler::sendAll(const void* buffer, size_t size) {
    size_t sent = 0;
    const char* buf = static_cast<const char*>(buffer);

    while (sent < size) {
        ssize_t s = send(clientFd, buf + sent, size - sent, 0);
        if (s <= 0) return false;
        sent += s;
    }
    return true;
}

void ConnectionHandler::handle() {
    ChatLogic chatLogic;

    MessageHeader header;
    if (!recvAll(&header, sizeof(header))) {
        close(clientFd);
        return;
    }

    std::string payload;
    if (header.payloadLength > 0) {
        payload.resize(header.payloadLength);
        if (!recvAll(payload.data(), header.payloadLength)) {
            close(clientFd);
            return;
        }
    }

    MessageType type = static_cast<MessageType>(header.messageType);

    if (type == MessageType::CHAT_DIRECT) {
        uint32_t receiverId;
        std::memcpy(&receiverId, payload.data(), sizeof(uint32_t));
        receiverId = ntohl(receiverId);

        std::string message = payload.substr(sizeof(uint32_t));

        std::string ack =
            chatLogic.handleDirectChat(
                header.senderId,
                receiverId,
                message
            );

        // Send ACK
        MessageHeader ackHeader{};
        ackHeader.messageType = static_cast<uint16_t>(MessageType::CHAT_DIRECT);
        ackHeader.payloadLength = ack.size();

        sendAll(&ackHeader, sizeof(ackHeader));
        sendAll(ack.data(), ack.size());
    }

    close(clientFd);
}
