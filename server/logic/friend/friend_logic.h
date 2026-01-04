#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../../db/repository/friend_repository.h"
#include "../../db/database.h"

struct SendFriendRequestResult {
    bool success;
    std::string message;
};

struct AcceptFriendRequestResult {
    bool success;
    std::string message;
    std::string senderUsername;  // For notification
    std::string senderDisplayName;
};

struct DeclineFriendRequestResult {
    bool success;
    std::string message;
};

struct RemoveFriendResult {
    bool success;
    std::string message;
};

struct FriendListData {
    std::vector<FriendInfo> friends;
    bool success;
};

struct PendingRequestsData {
    std::vector<PendingRequest> requests;
    bool success;
};

/**
 * FriendLogic
 *  - Handles business logic for friend system
 *  - Uses FriendRepository for database operations
 *  - Validates all friend operations
 */
class FriendLogic {
public:
    explicit FriendLogic(Database& db);

    // Send a friend request
    SendFriendRequestResult sendFriendRequest(uint32_t senderId, const std::string& targetUsername);

    // Accept a friend request
    AcceptFriendRequestResult acceptFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Decline a friend request
    DeclineFriendRequestResult declineFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Remove a friend
    RemoveFriendResult removeFriend(uint32_t userId, uint32_t friendId);

    // Get all friends
    FriendListData getFriendList(uint32_t userId);

    // Get all pending requests
    PendingRequestsData getPendingRequests(uint32_t userId);

private:
    FriendRepository repo;
};
