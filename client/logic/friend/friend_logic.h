#pragma once

#include <string>
#include <cstdint>
#include "../../net/friend/friend_sender.h"

/**
 * FriendLogic (Client-side)
 *  - Handles validation and processing of friend operations
 *  - Works with FriendSender to send friend messages to server
 */
class FriendLogic {
public:
    explicit FriendLogic(FriendSender& friendSender);

    // Handle send friend request with validation
    void handleSendFriendRequest(uint32_t userId, const std::string& targetUsername);

    // Handle accept friend request
    void handleAcceptFriendRequest(uint32_t userId, uint32_t senderId);

    // Handle decline friend request
    void handleDeclineFriendRequest(uint32_t userId, uint32_t senderId);

    // Handle remove friend
    void handleRemoveFriend(uint32_t userId, uint32_t friendId);

    // Handle request pending requests
    void handleRequestPendingRequests(uint32_t userId);

    // Handle request friend list
    void handleRequestFriendList(uint32_t userId);

private:
    FriendSender& friendSender_;
};
