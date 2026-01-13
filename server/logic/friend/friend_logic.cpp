#include "friend_logic.h"
#include <iostream>

FriendLogic::FriendLogic(Database& db) : repo(db) {
    std::cout << "[FriendLogic] Initialized\n";
}

SendFriendRequestResult FriendLogic::sendFriendRequest(uint32_t senderId, const std::string& targetUsername) {
    std::cout << "[FriendLogic] sendFriendRequest - senderId=" << senderId << ", target=" << targetUsername << "\n";

    SendFriendRequestResult result{false, ""};

    // Validate input
    if (targetUsername.empty()) {
        result.message = "Username cannot be empty";
        std::cout << "[FriendLogic] Empty username\n";
        return result;
    }

    if (targetUsername.length() > 40) {
        result.message = "Username too long";
        std::cout << "[FriendLogic] Username too long\n";
        return result;
    }

    // Validate user is not trying to add themselves
    std::string senderUsername = repo.getUsernameById(senderId);
    if (senderUsername == targetUsername) {
        result.message = "Cannot add yourself as a friend";
        std::cout << "[FriendLogic] Self-friend attempt by senderId=" << senderId << "\n";
        return result;
    }

    // Use repository to send request
    auto repoResult = repo.sendFriendRequest(senderId, targetUsername);
    result.success = repoResult.success;
    result.message = repoResult.message;

    std::cout << "[FriendLogic] sendFriendRequest result: success=" << result.success 
              << ", message=" << result.message << "\n";

    return result;
}

AcceptFriendRequestResult FriendLogic::acceptFriendRequest(uint32_t receiverId, uint32_t senderId) {
    std::cout << "[FriendLogic] acceptFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    AcceptFriendRequestResult result{false, "", "", ""};

    if (senderId == 0 || receiverId == 0) {
        result.message = "Invalid user ID";
        std::cout << "[FriendLogic] Invalid user ID\n";
        return result;
    }

    // Get sender info for notification
    result.senderUsername = repo.getUsernameById(senderId);
    result.senderDisplayName = repo.getDisplayName(senderId);

    auto repoResult = repo.acceptFriendRequest(receiverId, senderId);
    result.success = repoResult.success;
    result.message = repoResult.message;

    std::cout << "[FriendLogic] acceptFriendRequest result: success=" << result.success 
              << ", senderUsername=" << result.senderUsername << "\n";

    return result;
}

DeclineFriendRequestResult FriendLogic::declineFriendRequest(uint32_t receiverId, uint32_t senderId) {
    std::cout << "[FriendLogic] declineFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    DeclineFriendRequestResult result{false, ""};

    if (senderId == 0 || receiverId == 0) {
        result.message = "Invalid user ID";
        return result;
    }

    auto repoResult = repo.declineFriendRequest(receiverId, senderId);
    result.success = repoResult.success;
    result.message = repoResult.message;

    std::cout << "[FriendLogic] declineFriendRequest result: success=" << result.success << "\n";

    return result;
}

RemoveFriendResult FriendLogic::removeFriend(uint32_t userId, uint32_t friendId) {
    std::cout << "[FriendLogic] removeFriend - userId=" << userId << ", friendId=" << friendId << "\n";

    RemoveFriendResult result{false, ""};

    if (userId == 0 || friendId == 0) {
        result.message = "Invalid user ID";
        return result;
    }

    auto repoResult = repo.removeFriend(userId, friendId);
    result.success = repoResult.success;
    result.message = repoResult.message;

    std::cout << "[FriendLogic] removeFriend result: success=" << result.success << "\n";

    return result;
}

FriendListData FriendLogic::getFriendList(uint32_t userId) {
    std::cout << "[FriendLogic] getFriendList - userId=" << userId << "\n";

    FriendListData result;
    result.success = true;
    result.friends = repo.getUserFriends(userId);

    std::cout << "[FriendLogic] getFriendList returned " << result.friends.size() << " friends\n";

    return result;
}

PendingRequestsData FriendLogic::getPendingRequests(uint32_t userId) {
    std::cout << "[FriendLogic] getPendingRequests - userId=" << userId << "\n";

    PendingRequestsData result;
    result.success = true;
    result.requests = repo.getPendingRequests(userId);

    std::cout << "[FriendLogic] getPendingRequests returned " << result.requests.size() << " pending requests\n";

    return result;
}
