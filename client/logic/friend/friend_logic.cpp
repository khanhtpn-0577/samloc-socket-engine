#include "friend_logic.h"
#include <iostream>
#include <cctype>
#include <algorithm>

FriendLogic::FriendLogic(FriendSender& friendSender)
    : friendSender_(friendSender) {}

void FriendLogic::handleSendFriendRequest(uint32_t userId, const std::string& targetUsername) {
    std::cout << "[FriendLogic] handleSendFriendRequest: userId=" << userId << ", targetUsername=" << targetUsername << "\n";

    // Validation
    if (targetUsername.empty()) {
        std::cout << "[FriendLogic] Validation failed: empty targetUsername\n";
        return;
    }

    if (targetUsername.length() > 30) {
        std::cout << "[FriendLogic] Validation failed: targetUsername too long\n";
        return;
    }

    // Only alphanumeric and underscore
    for (char c : targetUsername) {
        if (!std::isalnum(c) && c != '_') {
            std::cout << "[FriendLogic] Validation failed: invalid character in username\n";
            return;
        }
    }

    std::cout << "[FriendLogic] Validation passed, sending friend request\n";
    friendSender_.sendFriendRequest(userId, targetUsername);
}

void FriendLogic::handleAcceptFriendRequest(uint32_t userId, uint32_t senderId) {
    std::cout << "[FriendLogic] handleAcceptFriendRequest: userId=" << userId << ", senderId=" << senderId << "\n";

    if (senderId == 0) {
        std::cout << "[FriendLogic] Validation failed: invalid senderId\n";
        return;
    }

    friendSender_.acceptFriendRequest(userId, senderId);
}

void FriendLogic::handleDeclineFriendRequest(uint32_t userId, uint32_t senderId) {
    std::cout << "[FriendLogic] handleDeclineFriendRequest: userId=" << userId << ", senderId=" << senderId << "\n";

    if (senderId == 0) {
        std::cout << "[FriendLogic] Validation failed: invalid senderId\n";
        return;
    }

    friendSender_.declineFriendRequest(userId, senderId);
}

void FriendLogic::handleRemoveFriend(uint32_t userId, uint32_t friendId) {
    std::cout << "[FriendLogic] handleRemoveFriend: userId=" << userId << ", friendId=" << friendId << "\n";

    if (friendId == 0) {
        std::cout << "[FriendLogic] Validation failed: invalid friendId\n";
        return;
    }

    friendSender_.removeFriend(userId, friendId);
}

void FriendLogic::handleRequestPendingRequests(uint32_t userId) {
    std::cout << "[FriendLogic] handleRequestPendingRequests: userId=" << userId << "\n";
    friendSender_.requestPendingRequests(userId);
}

void FriendLogic::handleRequestFriendList(uint32_t userId) {
    std::cout << "[FriendLogic] handleRequestFriendList: userId=" << userId << "\n";
    friendSender_.requestFriendList(userId);
}
