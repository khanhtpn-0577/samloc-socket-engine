#pragma once

#include <string>
#include <vector>
#include "../../net/protocol.h"
#include "../../logic/friend/friend_logic.h"
#include <functional>

/**
 * FriendHandler (Client-side)
 *  - Routes user actions to FriendLogic
 *  - Handles friend-related messages from server
 */
// Local event structs (decoupled from NetworkEvent)
struct FriendRequestSentEvent {
    bool success;
    std::string message;
};

struct PendingRequestsReceivedEvent {
    std::vector<std::pair<uint32_t, std::pair<std::string, std::pair<std::string, std::string>>>> requests;
};

struct FriendRequestAcceptedEvent {
    uint32_t senderId;
    std::string senderUsername;
    std::string senderDisplayName;
};

struct FriendRequestReceivedEvent {
    uint32_t senderId;
    std::string senderUsername;
    std::string senderDisplayName;
    std::string timestamp;
};

struct ErrorMessageEvent {
    std::string message;
    std::string color;
};

struct FriendRemovedEvent {
    bool success;
    std::string message;
};

struct FriendListReceivedEvent {
    std::vector<
        std::pair<uint32_t, std::pair<std::string, std::pair<double, bool>>>
    > friends;
};


class FriendHandler {
public:
    explicit FriendHandler(FriendLogic& logic);

    // === UI Action Methods (call FriendLogic) ===
    void onSendFriendRequestClicked(uint32_t userId, const std::string& targetUsername);
    void onAcceptRequestClicked(uint32_t userId, uint32_t senderId);
    void onDeclineRequestClicked(uint32_t userId, uint32_t senderId);
    void onRemoveFriendClicked(uint32_t userId, uint32_t friendId);
    void onRequestPendingRequests(uint32_t userId);
    void onRequestFriendList(uint32_t userId);

    // === Server Response Parsing Methods ===
    void onSendFriendRequestResponse(const Message& message);
    void onGetPendingRequestsResponse(const Message& message);
    void onAcceptFriendRequestResponse(const Message& message);
    void onDeclineFriendRequestResponse(const Message& message);
    void onRemoveFriendResponse(const Message& message);
    void onFriendRequestReceivedNotification(const Message& message);
    void onFriendRequestAcceptedNotification(const Message& message);
    void onGetFriendListResponse(const Message& message);

    // === Callbacks ===
    using MessageCallback = std::function<void(const std::string& message, const std::string& color)>;
    using PendingRequestsCallback = std::function<void(const std::vector<std::pair<uint32_t, std::pair<std::string, std::pair<std::string, std::string>>>>& requests)>;
    using FriendListCallback =
    std::function<void(
        const std::vector<
            std::pair<uint32_t, std::pair<std::string, std::pair<double, bool>>>
        >& friends
    )>;

    void setMessageCallback(MessageCallback cb);
    void setPendingRequestsCallback(PendingRequestsCallback cb);
    void setFriendListCallback(FriendListCallback cb);

private:
    FriendLogic& logic_;
    MessageCallback messageCallback_;
    PendingRequestsCallback pendingRequestsCallback_;
    FriendListCallback friendListCallback_;

    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);
    bool parseBoolField(const std::string& payload, const std::string& key);
    double parseDoubleField(const std::string& payload, const std::string& key);
};
