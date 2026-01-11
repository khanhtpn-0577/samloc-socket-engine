#include "friend_handler.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include "../session/session_manager.h"

FriendHandler::FriendHandler(FriendLogic& friendLogic)
    : friendLogic(friendLogic) {
    std::cout << "[FriendHandler] Initialized\n";
}

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

std::string FriendHandler::buildPayload(
    bool success,
    const std::string& message,
    const std::string& extraData
) {
    std::stringstream ss;
    ss << "{\"success\":" << (success ? "true" : "false")
       << ",\"message\":\"" << message << "\"";
    
    if (!extraData.empty()) {
        ss << "," << extraData;
    }
    
    ss << "}";
    return ss.str();
}

std::string FriendHandler::buildFriendListPayload(const std::vector<FriendData>& friends) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < friends.size(); i++) {
        ss << "{\"userId\":" << friends[i].userId
           << ",\"username\":\"" << friends[i].username
           << "\",\"balance\":" << std::fixed << std::setprecision(2) << friends[i].balance << "}";
        if (i < friends.size() - 1) ss << ",";
        
        std::cout << "[FriendHandler] Serializing friend: userId=" << friends[i].userId
                  << ", username=" << friends[i].username
                  << ", balance=" << friends[i].balance << "\n";
    }
    ss << "]";
    return ss.str();
}

std::string FriendHandler::buildPendingRequestsPayload(const std::vector<PendingRequest>& requests) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < requests.size(); i++) {
        // Format timestamp
        auto time_point = std::chrono::system_clock::from_time_t(requests[i].sentAt / 1000);
        auto time_t_val = std::chrono::system_clock::to_time_t(time_point);
        struct tm* tm_info = localtime(&time_t_val);
        
        std::stringstream timestamp_ss;
        timestamp_ss << std::put_time(tm_info, "%b %d, %I:%M %p");
        
        ss << "{\"senderId\":" << requests[i].senderId
           << ",\"senderUsername\":\"" << requests[i].senderUsername
           << "\",\"senderDisplayName\":\"" << requests[i].senderDisplayName
           << "\",\"timestamp\":\"" << timestamp_ss.str() << "\"}";
        if (i < requests.size() - 1) ss << ",";
    }
    ss << "]";
    return ss.str();
}

Message FriendHandler::createResponse(
    MessageType type,
    bool success,
    const std::string& message,
    const std::string& extraData
) {
    Message response;
    response.header.messageType = static_cast<uint16_t>(type);
    response.header.senderId = 0;  // Server
    response.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    response.payload = buildPayload(success, message, extraData);
    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);
    
    return response;
}

Message FriendHandler::handleSendFriendRequest(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling SEND_FRIEND_REQUEST\n";

    uint32_t senderId = incomingMsg.header.senderId;
    std::string targetUsername = parseField(incomingMsg.payload, "targetUsername");

    std::cout << "[FriendHandler] senderId=" << senderId << ", targetUsername=" << targetUsername << "\n";

    auto result = friendLogic.sendFriendRequest(senderId, targetUsername);

    return createResponse(
        MessageType::SEND_FRIEND_REQUEST_RESPONSE,
        result.success,
        result.message
    );
}

Message FriendHandler::handleAcceptFriendRequest(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling ACCEPT_FRIEND_REQUEST\n";

    uint32_t receiverId = incomingMsg.header.senderId;
    uint32_t senderId = parseUint32Field(incomingMsg.payload, "senderId");

    std::cout << "[FriendHandler] receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    auto result = friendLogic.acceptFriendRequest(receiverId, senderId);

    // Build response with sender info for notification
    std::string extraData = "";
    if (result.success) {
        extraData = std::string("\"senderUsername\":\"") + result.senderUsername + 
                    "\",\"senderDisplayName\":\"" + result.senderDisplayName + "\"";
    }

    return createResponse(
        MessageType::ACCEPT_FRIEND_REQUEST_RESPONSE,
        result.success,
        result.message,
        extraData
    );
}

Message FriendHandler::handleDeclineFriendRequest(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling DECLINE_FRIEND_REQUEST\n";

    uint32_t receiverId = incomingMsg.header.senderId;
    uint32_t senderId = parseUint32Field(incomingMsg.payload, "senderId");

    std::cout << "[FriendHandler] receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    auto result = friendLogic.declineFriendRequest(receiverId, senderId);

    return createResponse(
        MessageType::DECLINE_FRIEND_REQUEST_RESPONSE,
        result.success,
        result.message
    );
}

Message FriendHandler::handleRemoveFriend(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling REMOVE_FRIEND\n";

    uint32_t userId = incomingMsg.header.senderId;
    uint32_t friendId = parseUint32Field(incomingMsg.payload, "friendId");

    std::cout << "[FriendHandler] userId=" << userId << ", friendId=" << friendId << "\n";

    auto result = friendLogic.removeFriend(userId, friendId);

    return createResponse(
        MessageType::REMOVE_FRIEND_RESPONSE,
        result.success,
        result.message
    );
}

Message FriendHandler::handleGetPendingRequests(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling GET_PENDING_REQUESTS\n";

    uint32_t userId = incomingMsg.header.senderId;

    std::cout << "[FriendHandler] userId=" << userId << "\n";

    auto result = friendLogic.getPendingRequests(userId);

    Message response;
    response.header.messageType = static_cast<uint16_t>(MessageType::GET_PENDING_REQUESTS_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    response.payload = buildPayload(result.success, "Pending requests");
    
    // Append pending requests array
    std::string requestsArray = buildPendingRequestsPayload(result.requests);
    response.payload.insert(response.payload.length() - 1, ",\"requests\":" + requestsArray);
    
    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);

    return response;
}

Message FriendHandler::handleGetFriendList(const Message& incomingMsg) {
    std::cout << "[FriendHandler] Handling GET_FRIEND_LIST\n";

    uint32_t userId = incomingMsg.header.senderId;

    std::cout << "[FriendHandler] userId=" << userId << "\n";

    auto result = friendLogic.getFriendList(userId);

    Message response;
    response.header.messageType = static_cast<uint16_t>(MessageType::GET_FRIEND_LIST_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    response.payload = buildPayload(result.success, "Friend list");
    
    // Append friend list array
    //std::string friendsArray = buildFriendListPayload(result.friends);
    std::string friendsArray = buildFriendListPayloadWithOnline(result.friends);
    response.payload.insert(response.payload.length() - 1, ",\"friends\":" + friendsArray);
    
    response.header.payloadLength = response.payload.size();
    std::memset(response.header.token, 0, 32);

    return response;
}

std::string FriendHandler::buildFriendListPayloadWithOnline(
    const std::vector<FriendData>& friends) {

    std::ostringstream oss;
    oss << "[";

    for (size_t i = 0; i < friends.size(); ++i) {
        const auto& f = friends[i];

        bool online = (SessionManager::instance().get(f.userId) != nullptr);

        oss << "{"
            << "\"userId\":" << f.userId << ","
            << "\"username\":\"" << f.username << "\","
            << "\"balance\":" << f.balance << ","
            << "\"online\":" << (online ? "true" : "false")
            << "}";

        if (i + 1 < friends.size()) oss << ",";
    }

    oss << "]";
    return oss.str();
}