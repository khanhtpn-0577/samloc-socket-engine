#include "friend_repository.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

FriendRepository::FriendRepository(Database& db) : db(db) {
    std::cout << "[FriendRepository] Initialized\n";
}

std::string FriendRepository::formatTimestamp(int64_t timestamp) {
    // Convert Unix timestamp to "Jan 3, 2:30 PM" format
    auto time_point = std::chrono::system_clock::from_time_t(timestamp / 1000);
    auto time_t_val = std::chrono::system_clock::to_time_t(time_point);
    struct tm* tm_info = localtime(&time_t_val);
    
    std::stringstream ss;
    ss << std::put_time(tm_info, "%b %d, %I:%M %p");
    return ss.str();
}

bool FriendRepository::usernameExists(const std::string& username) {
    std::cout << "[FriendRepository] Checking if username exists: " << username << "\n";
    QueryResult result = db.queryPrepared(
        "SELECT player_id FROM players WHERE username = ?;",
        {username}
    );
    bool exists = !result.empty();
    std::cout << "[FriendRepository] Username exists: " << username << " -> " << (exists ? "YES" : "NO") << "\n";
    return exists;
}

uint32_t FriendRepository::getUserIdByUsername(const std::string& username) {
    std::cout << "[FriendRepository] Getting userId for username: " << username << "\n";
    QueryResult result = db.queryPrepared(
        "SELECT player_id FROM players WHERE username = ?;",
        {username}
    );
    if (result.empty()) {
        std::cout << "[FriendRepository] Username not found: " << username << "\n";
        return 0;
    }
    uint32_t userId = std::stoul(result[0]["player_id"]);
    std::cout << "[FriendRepository] userId for " << username << " = " << userId << "\n";
    return userId;
}

std::string FriendRepository::getUsernameById(uint32_t userId) {
    QueryResult result = db.queryPrepared(
        "SELECT username FROM players WHERE player_id = ?;",
        {std::to_string(userId)}
    );
    if (result.empty()) return "";
    return result[0]["username"];
}

std::string FriendRepository::getDisplayName(uint32_t userId) {
    QueryResult result = db.queryPrepared(
        "SELECT display_name FROM players WHERE player_id = ?;",
        {std::to_string(userId)}
    );
    if (result.empty()) return getUsernameById(userId);
    return result[0]["display_name"];
}

double FriendRepository::getBalance(uint32_t userId) {
    QueryResult result = db.queryPrepared(
        "SELECT balance FROM players WHERE player_id = ?;",
        {std::to_string(userId)}
    );
    if (result.empty()) return 0.0;
    return std::stod(result[0]["balance"]);
}

bool FriendRepository::areFriends(uint32_t userId1, uint32_t userId2) {
    std::cout << "[FriendRepository] Checking if " << userId1 << " and " << userId2 << " are friends\n";
    QueryResult result = db.queryPrepared(
        "SELECT 1 FROM friends WHERE player_id = ? AND friend_id = ?;",
        {std::to_string(userId1), std::to_string(userId2)}
    );
    bool isFriend = !result.empty();
    std::cout << "[FriendRepository] Are friends: " << (isFriend ? "YES" : "NO") << "\n";
    return isFriend;
}

bool FriendRepository::hasPendingRequest(uint32_t senderId, uint32_t receiverId) {
    std::cout << "[FriendRepository] Checking pending request from " << senderId << " to " << receiverId << "\n";
    QueryResult result = db.queryPrepared(
        "SELECT 1 FROM friend_requests WHERE sender_id = ? AND receiver_id = ? AND status = 'pending';",
        {std::to_string(senderId), std::to_string(receiverId)}
    );
    bool hasPending = !result.empty();
    std::cout << "[FriendRepository] Has pending request: " << (hasPending ? "YES" : "NO") << "\n";
    return hasPending;
}

std::string FriendRepository::getRequestStatus(uint32_t senderId, uint32_t receiverId) {
    QueryResult result = db.queryPrepared(
        "SELECT status FROM friend_requests WHERE sender_id = ? AND receiver_id = ? ORDER BY sent_at DESC LIMIT 1;",
        {std::to_string(senderId), std::to_string(receiverId)}
    );
    if (result.empty()) return "none";
    return result[0]["status"];
}

FriendRepository::SendRequestResult FriendRepository::sendFriendRequest(uint32_t senderId, const std::string& targetUsername) {
    SendRequestResult result{false, "", 0};

    std::cout << "[FriendRepository] sendFriendRequest - senderId=" << senderId << ", target=" << targetUsername << "\n";

    // Step 1: Check if target username exists
    if (!usernameExists(targetUsername)) {
        result.message = "Username not found";
        std::cout << "[FriendRepository] Target username not found: " << targetUsername << "\n";
        return result;
    }

    uint32_t receiverId = getUserIdByUsername(targetUsername);
    std::cout << "[FriendRepository] Target receiverId: " << receiverId << "\n";

    // Step 2: Check if they're already friends
    if (areFriends(senderId, receiverId)) {
        result.message = "Already friends";
        std::cout << "[FriendRepository] Already friends\n";
        return result;
    }

    // Step 3: Check if there's a pending request
    if (hasPendingRequest(senderId, receiverId)) {
        result.message = "Request already sent";
        std::cout << "[FriendRepository] Request already sent\n";
        return result;
    }

    // Step 4: Check previous declined request
    std::string lastStatus = getRequestStatus(senderId, receiverId);
    if (lastStatus == "declined") {
        std::cout << "[FriendRepository] Previous request was declined, allowing new request\n";
    }

    // Insert the request
    std::cout << "[FriendRepository] Inserting friend request\n";
    bool inserted = db.executePrepared(
        "INSERT INTO friend_requests (sender_id, receiver_id, status, sent_at) VALUES (?, ?, 'pending', CURRENT_TIMESTAMP);",
        {std::to_string(senderId), std::to_string(receiverId)}
    );

    if (!inserted) {
        result.message = "Database error";
        std::cout << "[FriendRepository] Failed to insert friend request\n";
        return result;
    }

    result.success = true;
    result.message = "Request sent";
    result.requestId = static_cast<uint32_t>(db.getLastInsertId());
    std::cout << "[FriendRepository] Friend request sent successfully: requestId=" << result.requestId << "\n";
    return result;
}

FriendRepository::AcceptResult FriendRepository::acceptFriendRequest(uint32_t receiverId, uint32_t senderId) {
    AcceptResult result{false, ""};

    std::cout << "[FriendRepository] acceptFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    // Update request status to accepted
    bool updated = db.executePrepared(
        "UPDATE friend_requests SET status = 'accepted' WHERE sender_id = ? AND receiver_id = ? AND status = 'pending';",
        {std::to_string(senderId), std::to_string(receiverId)}
    );

    if (!updated) {
        result.message = "Failed to accept request";
        std::cout << "[FriendRepository] Failed to update request status\n";
        return result;
    }

    // Create bidirectional friendship
    std::cout << "[FriendRepository] Creating bidirectional friendship\n";
    bool friend1Inserted = db.executePrepared(
        "INSERT INTO friends (player_id, friend_id, created_at) VALUES (?, ?, CURRENT_TIMESTAMP);",
        {std::to_string(receiverId), std::to_string(senderId)}
    );

    bool friend2Inserted = db.executePrepared(
        "INSERT INTO friends (player_id, friend_id, created_at) VALUES (?, ?, CURRENT_TIMESTAMP);",
        {std::to_string(senderId), std::to_string(receiverId)}
    );

    if (!friend1Inserted || !friend2Inserted) {
        result.message = "Database error";
        std::cout << "[FriendRepository] Failed to create friendship\n";
        return result;
    }

    result.success = true;
    result.message = "Request accepted";
    std::cout << "[FriendRepository] Friend request accepted successfully\n";
    return result;
}

FriendRepository::DeclineResult FriendRepository::declineFriendRequest(uint32_t receiverId, uint32_t senderId) {
    DeclineResult result{false, ""};

    std::cout << "[FriendRepository] declineFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    bool updated = db.executePrepared(
        "UPDATE friend_requests SET status = 'declined' WHERE sender_id = ? AND receiver_id = ? AND status = 'pending';",
        {std::to_string(senderId), std::to_string(receiverId)}
    );

    if (!updated) {
        result.message = "Failed to decline request";
        std::cout << "[FriendRepository] Failed to decline request\n";
        return result;
    }

    result.success = true;
    result.message = "Request declined";
    std::cout << "[FriendRepository] Friend request declined successfully\n";
    return result;
}

FriendRepository::RemoveResult FriendRepository::removeFriend(uint32_t userId, uint32_t friendId) {
    RemoveResult result{false, ""};

    std::cout << "[FriendRepository] removeFriend - userId=" << userId << ", friendId=" << friendId << "\n";

    bool deleted = db.executePrepared(
        "DELETE FROM friends WHERE player_id = ? AND friend_id = ?;",
        {std::to_string(userId), std::to_string(friendId)}
    );

    if (!deleted) {
        result.message = "Failed to remove friend";
        std::cout << "[FriendRepository] Failed to remove friend\n";
        return result;
    }

    result.success = true;
    result.message = "Friend removed";
    std::cout << "[FriendRepository] Friend removed successfully\n";
    return result;
}

std::vector<FriendInfo> FriendRepository::getUserFriends(uint32_t userId) {
    std::cout << "[FriendRepository] getUserFriends - userId=" << userId << "\n";

    std::vector<FriendInfo> friends;
    QueryResult result = db.queryPrepared(
        "SELECT p.player_id, p.username, p.balance, f.created_at "
        "FROM friends f "
        "JOIN players p ON f.friend_id = p.player_id "
        "WHERE f.player_id = ? "
        "ORDER BY f.created_at DESC;",
        {std::to_string(userId)}
    );

    std::cout << "[FriendRepository] Found " << result.size() << " friends\n";

    for (const auto& row : result) {
        FriendInfo info;
        info.userId = std::stoul(row.at("player_id"));
        info.username = row.at("username");
        info.balance = std::stod(row.at("balance"));
        info.createdAt = std::stoll(row.at("created_at"));
        friends.push_back(info);
    }

    return friends;
}

std::vector<PendingRequest> FriendRepository::getPendingRequests(uint32_t userId) {
    std::cout << "[FriendRepository] getPendingRequests - userId=" << userId << "\n";

    std::vector<PendingRequest> requests;
    QueryResult result = db.queryPrepared(
        "SELECT fr.sender_id, p.username, p.display_name, fr.sent_at "
        "FROM friend_requests fr "
        "JOIN players p ON fr.sender_id = p.player_id "
        "WHERE fr.receiver_id = ? AND fr.status = 'pending' "
        "ORDER BY fr.sent_at DESC;",
        {std::to_string(userId)}
    );

    std::cout << "[FriendRepository] Found " << result.size() << " pending requests\n";

    for (const auto& row : result) {
        PendingRequest req;
        req.senderId = std::stoul(row.at("sender_id"));
        req.senderUsername = row.at("username");
        req.senderDisplayName = row.at("display_name");
        req.sentAt = std::stoll(row.at("sent_at"));
        requests.push_back(req);
    }

    return requests;
}
