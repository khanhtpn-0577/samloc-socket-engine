#pragma once

#include "../../net/protocol.h"
#include "../../logic/friend/friend_logic.h"
#include <string>

/**
 * FriendHandler
 *  - Handles all friend-related messages
 *  - Calls FriendLogic for business logic
 *  - Constructs response messages
 */
class FriendHandler {
public:
    explicit FriendHandler(FriendLogic& friendLogic);

    // Handle SEND_FRIEND_REQUEST message
    Message handleSendFriendRequest(const Message& incomingMsg);

    // Handle ACCEPT_FRIEND_REQUEST message
    Message handleAcceptFriendRequest(const Message& incomingMsg);

    // Handle DECLINE_FRIEND_REQUEST message
    Message handleDeclineFriendRequest(const Message& incomingMsg);

    // Handle REMOVE_FRIEND message
    Message handleRemoveFriend(const Message& incomingMsg);

    // Handle GET_PENDING_REQUESTS message
    Message handleGetPendingRequests(const Message& incomingMsg);

    // Handle GET_FRIEND_LIST message
    Message handleGetFriendList(const Message& incomingMsg);

private:
    // Create response message
    Message createResponse(
        MessageType type,
        bool success,
        const std::string& message,
        const std::string& extraData = ""
    );

    // Parse simple JSON-like payload
    std::string parseField(const std::string& payload, const std::string& key);

    // Parse numeric field
    uint32_t parseUint32Field(const std::string& payload, const std::string& key);

    // Build simple JSON-like payload
    std::string buildPayload(
        bool success,
        const std::string& message,
        const std::string& extraData = ""
    );

    // Build JSON payload with friend list
    std::string buildFriendListPayload(const std::vector<FriendData>& friends);

    // Build JSON payload with pending requests
    std::string buildPendingRequestsPayload(const std::vector<PendingRequest>& requests);

    std::string buildFriendListPayloadWithOnline(const std::vector<FriendData>& friends);

    FriendLogic& friendLogic;
};
