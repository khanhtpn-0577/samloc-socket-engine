#include "friend_sender.h"

FriendSender::FriendSender(ClientSocket& socket) : socket(socket) {
    std::cout << "[FriendSender] Initialized\n";
}

std::string FriendSender::buildPayload(const std::string& data) {
    return "{" + data + "}";
}

void FriendSender::sendMessage(MessageType type, uint32_t senderId, const std::string& payload) {
    Message msg;
    msg.header.messageType = static_cast<uint16_t>(type);
    msg.header.senderId = senderId;
    msg.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    msg.payload = payload;
    msg.header.payloadLength = msg.payload.size();
    std::memset(msg.header.token, 0, 32);

    std::cout << "[FriendSender] Sending message type 0x" << std::hex << static_cast<uint16_t>(type) << std::dec << "\n";
    socket.send(msg.serialize());
}

void FriendSender::sendFriendRequest(uint32_t senderId, const std::string& targetUsername) {
    std::cout << "[FriendSender] sendFriendRequest - senderId=" << senderId << ", target=" << targetUsername << "\n";

    std::string payload = buildPayload("\"targetUsername\":\"" + targetUsername + "\"");
    sendMessage(MessageType::SEND_FRIEND_REQUEST, senderId, payload);
}

void FriendSender::acceptFriendRequest(uint32_t receiverId, uint32_t senderId) {
    std::cout << "[FriendSender] acceptFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    std::string payload = buildPayload("\"senderId\":" + std::to_string(senderId));
    sendMessage(MessageType::ACCEPT_FRIEND_REQUEST, receiverId, payload);
}

void FriendSender::declineFriendRequest(uint32_t receiverId, uint32_t senderId) {
    std::cout << "[FriendSender] declineFriendRequest - receiverId=" << receiverId << ", senderId=" << senderId << "\n";

    std::string payload = buildPayload("\"senderId\":" + std::to_string(senderId));
    sendMessage(MessageType::DECLINE_FRIEND_REQUEST, receiverId, payload);
}

void FriendSender::removeFriend(uint32_t userId, uint32_t friendId) {
    std::cout << "[FriendSender] removeFriend - userId=" << userId << ", friendId=" << friendId << "\n";

    std::string payload = buildPayload("\"friendId\":" + std::to_string(friendId));
    sendMessage(MessageType::REMOVE_FRIEND, userId, payload);
}

void FriendSender::requestPendingRequests(uint32_t userId) {
    std::cout << "[FriendSender] requestPendingRequests - userId=" << userId << "\n";

    std::string payload = buildPayload("");
    sendMessage(MessageType::GET_PENDING_REQUESTS, userId, payload);
}
void FriendSender::requestFriendList(uint32_t userId) {
    std::cout << "[FriendSender] requestFriendList - userId=" << userId << "\n";

    std::string payload = buildPayload("");
    sendMessage(MessageType::GET_FRIEND_LIST, userId, payload);
}