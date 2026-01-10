#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

enum class MessageType : uint16_t {
    CHAT_DIRECT = 0x0001,
    CHAT_ROOM = 0x0002,
    SERVER_DELIVER_MESSAGE = 0x0003,

    JOIN_ROOM = 0x0010,
    PLAY_CARDS = 0x0020,
    PASS_ROUND = 0x0021,
    DECLARE_SAM = 0x0022,
    SPIN_WHEEL = 0x0030,

    FRIEND_LIST_REQUEST = 0x0040,
    FRIEND_LIST_RESPONSE = 0x0041,
    PRIVATE_CHAT_HISTORY_REQUEST = 0x0042,
    PRIVATE_CHAT_HISTORY_RESPONSE = 0x0043,
    //Rank messages
    FRIEND_RANK_REQUEST = 0x0044,
    FRIEND_RANK_RESPONSE = 0x0045,
    // Friend messages
    SEND_FRIEND_REQUEST = 0x0050,
    SEND_FRIEND_REQUEST_RESPONSE = 0x0051,
    GET_PENDING_REQUESTS = 0x0052,
    GET_PENDING_REQUESTS_RESPONSE = 0x0053,
    ACCEPT_FRIEND_REQUEST = 0x0054,
    ACCEPT_FRIEND_REQUEST_RESPONSE = 0x0055,
    DECLINE_FRIEND_REQUEST = 0x0056,
    DECLINE_FRIEND_REQUEST_RESPONSE = 0x0057,
    REMOVE_FRIEND = 0x0058,
    REMOVE_FRIEND_RESPONSE = 0x0059,
    FRIEND_REQUEST_RECEIVED_NOTIFICATION = 0x005A,
    FRIEND_REQUEST_ACCEPTED_NOTIFICATION = 0x005B,
    GET_FRIEND_LIST = 0x005C,
    GET_FRIEND_LIST_RESPONSE = 0x005D,

    //Lucky Wheel messages
    LUCKY_WHEEL_SPIN_REQUEST = 0x0060,
    LUCKY_WHEEL_SPIN_RESPONSE = 0x0061,
    // Auth messages
    SIGNUP = 0x0100,
    SIGNUP_RESPONSE = 0x0101,
    LOGIN = 0x0102,
    LOGIN_RESPONSE = 0x0103,
    LOGOUT = 0x0104,
    LOGOUT_RESPONSE = 0x0105,
    REQUEST_BALANCE = 0x0106,
    REQUEST_BALANCE_RESPONSE = 0x0107,


    C_LEAVE_ROOM = 0x200C,     
    
    SEND_CHALLENGE = 0x0200,
    SEND_CHALLENGE_RESPONSE = 0x0201,
    ACCEPT_CHALLENGE = 0x0202,
    ACCEPT_CHALLENGE_RESPONSE = 0x0203,
    REJECT_CHALLENGE = 0x0204,
    REJECT_CHALLENGE_RESPONSE = 0x0205,
    CANCEL_CHALLENGE = 0x0206,
    CANCEL_CHALLENGE_RESPONSE = 0x0207,
    CHALLENGE_NOTIFICATION = 0x0208,
    CHALLENGE_EXPIRED = 0x0209,
    
    CHAT_DIRECT_ACK = 0x8001,

    C_GET_ROOM_LIST = 0x2000,
    S_ROOM_LIST = 0x2001,
    C_JOIN_ROOM = 0x2002,
    S_JOIN_ROOM_FAIL = 0x200A,

    S_PLAYER_JOINED = 0x2004,
    S_PLAYER_LEFT = 0x2005,
    C_READY = 0x2006,
    S_PLAYER_READY = 0x2007, // Server không dùng, S_ROOM_UPDATE bao quát
    S_ROOM_UPDATE = 0x2008,
    S_EXISTING_PLAYERS = 0x2009,

    S_GAME_START_COUNTDOWN = 0x200B,
    S_GAME_START = 0x3001,
    S_TURN_INFO = 0x3010, // Server báo ai đang đến lượt
    C_PLAY_CARD = 0x3011,
    C_PASS_TURN = 0x3012,
    S_MOVE_RESULT = 0x3013, // Server báo 1 người vừa đánh/pass
    S_GAME_END = 0x3020,
    C_CREATE_PRIVATE_ROOM = 0x4000,
    S_CREATE_PRIVATE_ROOM_RESPONSE = 0x4001,
    
    ERROR_MESSAGE = 0x0090,
    PLAYER_DISCONNECT_GAME = 0x0091,
    S_JOIN_ROOM_SUCCESS = 0x2003
};

#pragma pack(push, 1)
struct MessageHeader {
    uint16_t messageType;
    uint32_t senderId;
    uint64_t timestamp;
    uint8_t token[32];
    uint32_t payloadLength;
    
    MessageHeader() : messageType(0), senderId(0), timestamp(0), payloadLength(0) {
        std::memset(token, 0, 32);
    }
};
#pragma pack(pop)

struct Message {
    MessageHeader header;
    std::string payload;
    
    Message() = default;
    
    std::string serialize() const {
        std::string result;
        const char* headerPtr = reinterpret_cast<const char*>(&header);
        result.append(headerPtr, sizeof(MessageHeader));
        result.append(payload);
        return result;
    }
    
    static Message deserialize(const std::string& data) {
        Message msg;
        if (data.size() < sizeof(MessageHeader)) return msg;
        std::memcpy(&msg.header, data.data(), sizeof(MessageHeader));
        if (data.size() > sizeof(MessageHeader)) msg.payload = data.substr(sizeof(MessageHeader));
        return msg;
    }
};

inline void setToken(MessageHeader& header, const std::string& token) {
    size_t copySize = (token.size() < 32) ? token.size() : 32;
    std::memcpy(header.token, token.c_str(), copySize);
}

struct FriendInfo {
    uint32_t userId;
    std::string username;
};

struct ChatHistoryItem {
    uint32_t senderId;
    std::string content;
    std::string sentAt;
};