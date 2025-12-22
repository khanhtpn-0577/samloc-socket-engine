#include "connection_handler.h"

#include "../chat/chat_handler.h"
#include "../../logic/chat/chat_logic.h"
#include "../session/session_manager.h"

#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(int fd)
    : clientFd(fd), boundUserId(0) {}

int ConnectionHandler::getFd() const {
    return clientFd;
}

bool ConnectionHandler::recvAll(void* buffer, size_t size) {
    size_t received = 0;
    char* buf = static_cast<char*>(buffer);

    while (received < size) {
        ssize_t r = recv(clientFd, buf + received, size - received, 0);
        if (r == 0) {
            std::cout << "[Server] recv EOF (peer closed) fd=" << clientFd << "\n";
            return false;
        }
        if (r < 0) {
            std::cout << "[Server] recv error fd=" << clientFd
                    << " errno=" << errno
                    << " (" << std::strerror(errno) << ")\n";
            return false;
        }
        received += r;
    }
    return true;
}

bool ConnectionHandler::sendAll(const char* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t s = send(clientFd, data + sent, size - sent, 0);
        if (s < 0) {
            if (errno == EINTR) continue;
            std::cerr << "[Server] send error fd=" << clientFd
                      << " errno=" << errno
                      << " (" << strerror(errno) << ")\n";
            return false;
        }
        if (s == 0) {
            std::cerr << "[Server] send returned 0 fd=" << clientFd << "\n";
            return false;
        }
        sent += static_cast<size_t>(s);
    }
    return true;
}


bool ConnectionHandler::sendMessage(const Message& msg) {
    std::string bytes = msg.serialize();
    return sendAll(bytes.data(), bytes.size());
}


/**
 * Xử lý 1 event READ
 * return false => client disconnect
 */
bool ConnectionHandler::onReadable() {
    MessageHeader header;
    if (!recvAll(&header, sizeof(header))) {
        return false;
    }

    std::string payload;
    if (header.payloadLength > 0) {
        payload.resize(header.payloadLength);
        if (!recvAll(payload.data(), header.payloadLength)) {
            return false;
        }
    }

    Message incoming{header, payload};

    // bind session lần đầu
    if (boundUserId == 0 && header.senderId != 0) {
        boundUserId = header.senderId;
        SessionManager::instance().add(boundUserId, this);
        std::cout << "[Server] User " << boundUserId << " bound to fd "
                  << clientFd << std::endl;
    }

    ChatLogic chatLogic;
    ChatHandler chatHandler(chatLogic);

    Message response;

    switch (static_cast<MessageType>(header.messageType)) {
    case MessageType::CHAT_DIRECT:
        response = chatHandler.handleChatDirect(incoming);
        break;

    default:
        std::cerr << "[Server] Unsupported message type: "
                  << header.messageType << std::endl;
        return true;
    }

    // ===== log nội dung response =====
    std::cout << "[Server] Sending response"
              << " toFd=" << clientFd
              << " type=" << static_cast<int>(response.header.messageType)
              << " senderId=" << response.header.senderId
              << " payloadLen=" << response.header.payloadLength
              << std::endl;

    if (!response.payload.empty()) {
        std::cout << "[Server] Response payload: "
                  << response.payload << std::endl;
    }

    // ===== gửi response =====
    bool sendOk = sendMessage(response);

    if (sendOk) {
        std::cout << "[Server] Response sent successfully"
                  << " fd=" << clientFd << std::endl;
    } else {
        std::cerr << "[Server] Failed to send response"
                  << " fd=" << clientFd << std::endl;
        return false; // coi như connection lỗi
    }
    return true;
}

void ConnectionHandler::closeConnection() {
    if (boundUserId != 0) {
        SessionManager::instance().remove(boundUserId);
        std::cout << "[Server] User " << boundUserId << " unbound\n";
    }
    close(clientFd);
}
