#pragma once

#include "../client_socket.h"
#include "../protocol.h"
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>

/**
 * FriendSender
 *  - Sends friend-related messages to the server
 *  - Used by FriendsState to manage friend operations
 */
class FriendSender {
public:
    explicit FriendSender(ClientSocket& socket);

    // Send a friend request
    void sendFriendRequest(uint32_t senderId, const std::string& targetUsername);

    // Accept a friend request
    void acceptFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Decline a friend request
    void declineFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Remove a friend
    void removeFriend(uint32_t userId, uint32_t friendId);

    // Request pending requests list
    void requestPendingRequests(uint32_t userId);

    // Request friend list
    void requestFriendList(uint32_t userId);

private:
    ClientSocket& socket;

    // Helper: build JSON payload
    std::string buildPayload(const std::string& data);

    // Helper: send message
    void sendMessage(MessageType type, uint32_t senderId, const std::string& payload);
};
