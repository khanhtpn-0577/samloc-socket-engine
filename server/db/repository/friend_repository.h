#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../database.h"

struct FriendData {
    uint32_t userId;
    std::string username;
    std::string displayName;
    double balance;
    int64_t createdAt;
};

struct PendingRequest {
    uint32_t senderId;
    std::string senderUsername;
    std::string senderDisplayName;
    int64_t sentAt;
};

/**
 * FriendRepository
 *  - Handles all database operations for friend system
 *  - Manages friend_requests and friends tables
 */
class FriendRepository {
public:
    explicit FriendRepository(Database& db);

    // Send a friend request
    // Returns: {success, message, requestId if success}
    struct SendRequestResult {
        bool success;
        std::string message;
        uint32_t requestId;
    };
    SendRequestResult sendFriendRequest(uint32_t senderId, const std::string& targetUsername);

    // Accept a friend request
    struct AcceptResult {
        bool success;
        std::string message;
    };
    AcceptResult acceptFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Decline a friend request
    struct DeclineResult {
        bool success;
        std::string message;
    };
    DeclineResult declineFriendRequest(uint32_t receiverId, uint32_t senderId);

    // Remove friend (one-way)
    struct RemoveResult {
        bool success;
        std::string message;
    };
    RemoveResult removeFriend(uint32_t userId, uint32_t friendId);

    // Get all friends of a user (order by newest first)
    std::vector<FriendData> getUserFriends(uint32_t userId);

    // Get all pending friend requests for a user
    std::vector<PendingRequest> getPendingRequests(uint32_t userId);

    // Check if there's a pending request from sender to receiver
    bool hasPendingRequest(uint32_t senderId, uint32_t receiverId);

    // Check if two users are already friends
    bool areFriends(uint32_t userId1, uint32_t userId2);

    // Get username by userId
    std::string getUsernameById(uint32_t userId);

    // Get userId by username
    uint32_t getUserIdByUsername(const std::string& username);

    // Check if username exists
    bool usernameExists(const std::string& username);

    // Get user's display name
    std::string getDisplayName(uint32_t userId);

    // Get user's balance
    double getBalance(uint32_t userId);

private:
    Database& db;

    // Helper: format timestamp to "Jan 3, 2:30 PM"
    std::string formatTimestamp(int64_t timestamp);

    // Helper: check if request status is pending
    std::string getRequestStatus(uint32_t senderId, uint32_t receiverId);
};
