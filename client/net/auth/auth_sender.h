#pragma once

#include <string>
#include <cstdint>
#include "../protocol.h"
#include "../client_socket.h"

class AuthSender {
public:
    AuthSender(ClientSocket& socket, uint32_t userId, const std::string& token);

    bool sendSignup(const std::string& username, const std::string& password, const std::string& displayName);
    bool sendLogin(const std::string& username, const std::string& password);
    bool sendLogout();
    bool requestBalance(uint32_t userId);

    void updateIdentity(uint32_t newUserId, const std::string& newToken);

private:
    ClientSocket& socket;
    uint32_t userId;
    std::string token;

    static uint64_t getCurrentTimestamp();
    Message createMessage(MessageType type, const std::string& payload);
    bool sendMessage(MessageType type, const std::string& payload);
};
