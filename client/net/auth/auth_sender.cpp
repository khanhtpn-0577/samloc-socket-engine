#include "auth_sender.h"
#include <sstream>
#include <chrono>
#include <iostream>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif

AuthSender::AuthSender(ClientSocket& socket, uint32_t userId, const std::string& token)
    : socket(socket), userId(userId), token(token) {}

uint64_t AuthSender::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

Message AuthSender::createMessage(MessageType type, const std::string& payload) {
    Message msg;
    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId = userId;
    msg.header.timestamp = getCurrentTimestamp();
    msg.header.payloadLength = payload.size();

    setToken(msg.header, token);
    msg.payload = payload;
    return msg;
}

bool AuthSender::sendMessage(MessageType type, const std::string& payload) {
    Message msg = createMessage(type, payload);
    if (socket.sendMessage(msg)) {
        std::cout << "Message sent (type: " << static_cast<uint16_t>(type) << ")\n";
        return true;
    }
    std::cerr << "Failed to send message\n";
    return false;
}

bool AuthSender::sendSignup(const std::string& username, const std::string& password, const std::string& displayName) {
    std::stringstream payload;
    payload << "{\"u\":\"" << username << "\",\"p\":\"" << password << "\",\"d\":\"" << displayName << "\"}";
    if (sendMessage(MessageType::SIGNUP, payload.str())) {
        std::cout << "Signup request sent for user: " << username << "\n";
        return true;
    }
    return false;
}

bool AuthSender::sendLogin(const std::string& username, const std::string& password) {
    std::stringstream payload;
    payload << "{\"u\":\"" << username << "\",\"p\":\"" << password << "\"}";
    if (sendMessage(MessageType::LOGIN, payload.str())) {
        std::cout << "Login request sent for user: " << username << "\n";
        return true;
    }
    return false;
}

bool AuthSender::sendLogout() {
    if (sendMessage(MessageType::LOGOUT, "")) {
        std::cout << "Logout request sent\n";
        return true;
    }
    return false;
}

void AuthSender::updateIdentity(uint32_t newUserId, const std::string& newToken) {
    userId = newUserId;
    token = newToken;
}
