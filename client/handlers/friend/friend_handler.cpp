#include "friend_handler.h"
#include <iostream>
#include <sstream>

FriendHandler::FriendHandler(FriendLogic& logic)
    : logic_(logic) {}

// === UI Action Methods (call FriendLogic) ===

void FriendHandler::onSendFriendRequestClicked(uint32_t userId, const std::string& targetUsername) {
    std::cout << "[FriendHandler] onSendFriendRequestClicked: targetUsername=" << targetUsername << "\n";
    logic_.handleSendFriendRequest(userId, targetUsername);
}

void FriendHandler::onAcceptRequestClicked(uint32_t userId, uint32_t senderId) {
    std::cout << "[FriendHandler] onAcceptRequestClicked: senderId=" << senderId << "\n";
    logic_.handleAcceptFriendRequest(userId, senderId);
}

void FriendHandler::onDeclineRequestClicked(uint32_t userId, uint32_t senderId) {
    std::cout << "[FriendHandler] onDeclineRequestClicked: senderId=" << senderId << "\n";
    logic_.handleDeclineFriendRequest(userId, senderId);
}

void FriendHandler::onRemoveFriendClicked(uint32_t userId, uint32_t friendId) {
    std::cout << "[FriendHandler] onRemoveFriendClicked: friendId=" << friendId << "\n";
    logic_.handleRemoveFriend(userId, friendId);
}

void FriendHandler::onRequestPendingRequests(uint32_t userId) {
    std::cout << "[FriendHandler] onRequestPendingRequests\n";
    logic_.handleRequestPendingRequests(userId);
}

void FriendHandler::onRequestFriendList(uint32_t userId) {
    std::cout << "[FriendHandler] onRequestFriendList\n";
    logic_.handleRequestFriendList(userId);
}

// === Server Response Parsing Methods ===
std::string FriendHandler::parseField(const std::string& payload, const std::string& key) {
    std::string searchKey = "\"" + key + "\":\"";
    size_t keyPos = payload.find(searchKey);

    if (keyPos == std::string::npos) {
        return "";
    }

    size_t valueStart = keyPos + searchKey.length();
    size_t valueEnd = payload.find("\"", valueStart);

    if (valueEnd == std::string::npos) {
        return "";
    }

    return payload.substr(valueStart, valueEnd - valueStart);
}

uint32_t FriendHandler::parseUint32Field(const std::string& payload, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);

    if (keyPos == std::string::npos) {
        return 0;
    }

    size_t valueStart = keyPos + searchKey.length();
    size_t valueEnd = payload.find_first_of(",}", valueStart);

    if (valueEnd == std::string::npos) {
        return 0;
    }

    std::string valueStr = payload.substr(valueStart, valueEnd - valueStart);
    return std::stoul(valueStr);
}

bool FriendHandler::parseBoolField(const std::string& payload, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);

    if (keyPos == std::string::npos) {
        return false;
    }

    size_t valueStart = keyPos + searchKey.length();
    std::string rest = payload.substr(valueStart);
    return rest.find("true") == 0;
}

double FriendHandler::parseDoubleField(const std::string& payload, const std::string& key) {
    std::string searchKey = "\"" + key + "\":";
    size_t keyPos = payload.find(searchKey);

    if (keyPos == std::string::npos) {
        return 0.0;
    }

    size_t valueStart = keyPos + searchKey.length();
    size_t valueEnd = payload.find_first_of(",}", valueStart);

    if (valueEnd == std::string::npos) {
        return 0.0;
    }

    std::string valueStr = payload.substr(valueStart, valueEnd - valueStart);
    try {
        return std::stod(valueStr);
    } catch (...) {
        return 0.0;  // Default to 0.0 if malformed
    }
}

// === Callback setters ===
void FriendHandler::setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
}

void FriendHandler::setPendingRequestsCallback(PendingRequestsCallback cb) {
    pendingRequestsCallback_ = std::move(cb);
}

void FriendHandler::setFriendListCallback(FriendListCallback cb) {
    friendListCallback_ = std::move(cb);
}

// === Respons handlers with callbacks ===
void FriendHandler::onSendFriendRequestResponse(const Message& message) {
    std::cout << "[FriendHandler] onSendFriendRequestResponse\n";

    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");

    std::cout << "[FriendHandler] success=" << success << ", message=" << msg << "\n";

    if (messageCallback_) {
        messageCallback_(msg, success ? "green" : "red");
    }
}

void FriendHandler::onGetPendingRequestsResponse(const Message& message) {
    std::cout << "[FriendHandler] onGetPendingRequestsResponse\n";

    std::vector<std::pair<uint32_t, std::pair<std::string, std::pair<std::string, std::string>>>> requests;

    // Parse the requests array
    size_t requestsPos = message.payload.find("\"requests\":");
    if (requestsPos == std::string::npos) {
        std::cout << "[FriendHandler] No requests array found\n";
        if (pendingRequestsCallback_) {
            pendingRequestsCallback_(requests);
        }
        return;
    }

    size_t arrayStart = message.payload.find("[", requestsPos);
    size_t arrayEnd = message.payload.find("]", arrayStart);

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cout << "[FriendHandler] Invalid requests array\n";
        if (pendingRequestsCallback_) {
            pendingRequestsCallback_(requests);
        }
        return;
    }

    std::string arrayStr = message.payload.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    // Parse individual request objects
    size_t pos = 0;
    while (pos < arrayStr.length()) {
        size_t objStart = arrayStr.find("{", pos);
        if (objStart == std::string::npos) break;

        size_t objEnd = arrayStr.find("}", objStart);
        if (objEnd == std::string::npos) break;

        std::string objStr = arrayStr.substr(objStart, objEnd - objStart + 1);

        uint32_t senderId = parseUint32Field(objStr, "senderId");
        std::string username = parseField(objStr, "senderUsername");
        std::string displayName = parseField(objStr, "senderDisplayName");
        std::string timestamp = parseField(objStr, "timestamp");

        requests.push_back({
            senderId,
            {username, {displayName, timestamp}}
        });

        std::cout << "[FriendHandler] Parsed request: senderId=" << senderId << ", username=" << username << "\n";

        pos = objEnd + 1;
    }

    std::cout << "[FriendHandler] Total pending requests: " << requests.size() << "\n";

    if (pendingRequestsCallback_) {
        pendingRequestsCallback_(requests);
    }
}

void FriendHandler::onAcceptFriendRequestResponse(const Message& message) {
    std::cout << "[FriendHandler] onAcceptFriendRequestResponse\n";

    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");

    std::cout << "[FriendHandler] success=" << success << ", message=" << msg << "\n";

    if (messageCallback_) {
        messageCallback_(msg, success ? "green" : "red");
    }
}

void FriendHandler::onDeclineFriendRequestResponse(const Message& message) {
    std::cout << "[FriendHandler] onDeclineFriendRequestResponse\n";

    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");

    if (messageCallback_) {
        messageCallback_(msg, success ? "yellow" : "red");
    }
}

void FriendHandler::onRemoveFriendResponse(const Message& message) {
    std::cout << "[FriendHandler] onRemoveFriendResponse\n";

    bool success = parseBoolField(message.payload, "success");
    std::string msg = parseField(message.payload, "message");

    std::cout << "[FriendHandler] success=" << success << ", message=" << msg << "\n";

    if (messageCallback_) {
        messageCallback_(msg, success ? "green" : "red");
    }
}

void FriendHandler::onFriendRequestReceivedNotification(const Message& message) {
    std::cout << "[FriendHandler] onFriendRequestReceivedNotification\n";

    std::string senderDisplayName = parseField(message.payload, "senderDisplayName");
    std::string msg = senderDisplayName + " sent you a friend request";

    std::cout << "[FriendHandler] " << msg << "\n";

    if (messageCallback_) {
        messageCallback_(msg, "yellow");
    }
}

void FriendHandler::onFriendRequestAcceptedNotification(const Message& message) {
    std::cout << "[FriendHandler] onFriendRequestAcceptedNotification\n";

    std::string senderUsername = parseField(message.payload, "senderUsername");
    std::string msg = senderUsername + " accepted your friend request";

    std::cout << "[FriendHandler] " << msg << "\n";

    if (messageCallback_) {
        messageCallback_(msg, "green");
    }
}

void FriendHandler::onGetFriendListResponse(const Message& message) {
    std::cout << "[FriendHandler] onGetFriendListResponse\n";

    std::vector<std::pair<uint32_t, std::pair<std::string, std::pair<double, bool>>>> friends;
    //std::vector<FriendListEntry> friends;

    // Parse the friends array
    size_t friendsPos = message.payload.find("\"friends\":");
    if (friendsPos == std::string::npos) {
        std::cout << "[FriendHandler] No friends array found\n";
        if (friendListCallback_) {
            friendListCallback_(friends);
        }
        return;
    }

    size_t arrayStart = message.payload.find("[", friendsPos);
    size_t arrayEnd = message.payload.find("]", arrayStart);

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cout << "[FriendHandler] Invalid friends array\n";
        if (friendListCallback_) {
            friendListCallback_(friends);
        }
        return;
    }

    std::string arrayStr = message.payload.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    // Parse individual friend objects
    size_t pos = 0;
    while (pos < arrayStr.length()) {
        size_t objStart = arrayStr.find("{", pos);
        if (objStart == std::string::npos) break;

        size_t objEnd = arrayStr.find("}", objStart);
        if (objEnd == std::string::npos) break;

        std::string objStr = arrayStr.substr(objStart, objEnd - objStart + 1);

        uint32_t userId = parseUint32Field(objStr, "userId");
        std::string username = parseField(objStr, "username");
        std::string displayName = parseField(objStr, "displayName");
        double balance = parseDoubleField(objStr, "balance");
        bool online = parseBoolField(objStr, "online");

        friends.push_back({
            userId,
            {username, {balance, online}}
        });

        std::cout << "[FriendHandler] Parsed friend: userId=" << userId 
                  << ", username=" << username 
                  << ", balance=" << balance << ", online=" << online << "\n";

        pos = objEnd + 1;
    }

    std::cout << "[FriendHandler] Total friends: " << friends.size() << "\n";

    if (friendListCallback_) {
        friendListCallback_(friends);
    }
}
