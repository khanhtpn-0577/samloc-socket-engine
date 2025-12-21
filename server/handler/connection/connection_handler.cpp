#include "connection_handler.h"
#include "../../net/protocol.h"

#include "../chat/chat_handler.h"
#include "../../logic/chat/chat_logic.h"
#include "../session/session_manager.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>   // recv, send
#include <unistd.h>       // close
#include <arpa/inet.h>    // ntohl, htonl
#include <cstring>
#include <iostream>


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

bool ConnectionHandler::sendMessage(const Message& msg) {
    std::string bytes = msg.serialize();
    return sendAll(bytes.data(), bytes.size());
}


void ConnectionHandler::handle() {
    // ===== receive header =====
    MessageHeader header;
    if (!recvAll(&header, sizeof(header))) {
        close(clientFd);
        return;
    }

    // ===== receive payload =====
    std::string payload;
    if (header.payloadLength > 0) {
        payload.resize(header.payloadLength);
        if (!recvAll(payload.data(), header.payloadLength)) {
            close(clientFd);
            return;
        }
    }

    Message incomingMsg;
    incomingMsg.header = header;
    incomingMsg.payload = payload;

    MessageType type =
        static_cast<MessageType>(header.messageType);

    // =====REGISTER SESSION (QUAN TRỌNG) =====
    uint32_t senderId = incomingMsg.header.senderId;
    if (senderId != 0) {
        SessionManager::instance().add(senderId, this);
    }

    // ===== create logic & handlers =====
    ChatLogic chatLogic;
    ChatHandler chatHandler(chatLogic);

    Message response;


    // ===== dispatch =====
    switch (type) {
    case MessageType::CHAT_DIRECT:
        response = chatHandler.handleChatDirect(incomingMsg);
        break;

    default:
        std::cerr << "Unsupported message type\n";
        close(clientFd);
        return;
    }

    // ===== send response(ACK) =====
    std::string bytes = response.serialize();
    sendAll(bytes.data(), bytes.size());

    // =====UNREGISTER SESSION =====
    SessionManager::instance().remove(senderId);

    close(clientFd);
}
