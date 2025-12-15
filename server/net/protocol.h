#pragma once

#include <cstdint>
#include <string>
#include <cstring>

// Message Types
enum class MessageType : uint16_t {
    CHAT_DIRECT = 0x0001,
    CHAT_ROOM = 0x0002,
    JOIN_ROOM = 0x0010,
    LEAVE_ROOM = 0x0011,
    PLAY_CARDS = 0x0020,
    PASS_ROUND = 0x0021,
    DECLARE_SAM = 0x0022,
    SPIN_WHEEL = 0x0030,
    LOGIN = 0x0100,
    LOGOUT = 0x0101,
    CHAT_DIRECT_ACK = 0x8001,
};

// Message Header Structure
#pragma pack(push, 1)
struct MessageHeader {
    uint16_t messageType;          // 2 bytes
    uint32_t senderId;             // 4 bytes
    uint64_t timestamp;            // 8 bytes
    uint8_t token[32];             // 32 bytes (fixed)
    uint32_t payloadLength;        // 4 bytes
    
    // Total: 50 bytes
    
    //constructor
    MessageHeader() : messageType(0), senderId(0), timestamp(0), payloadLength(0) {
        std::memset(token, 0, 32);
    }
};
#pragma pack(pop)

// Complete Message Structure
struct Message {
    MessageHeader header;
    std::string payload;  // Variable length data
    
    Message() = default; //constructor mac dinh
    
    // Serialize message to bytes
    std::string serialize() const {
        std::string result;
        
        // Add header (50 bytes)
        const char* headerPtr = reinterpret_cast<const char*>(&header);
        result.append(headerPtr, sizeof(MessageHeader));
        
        // Add payload
        result.append(payload);
        
        return result;
    }
    
    // Deserialize from bytes
    static Message deserialize(const std::string& data) {
        Message msg;
        
        if (data.size() < sizeof(MessageHeader)) {
            return msg;  // neu du lieu < do dai header --> invalid
        }
        
        // Extract header
        std::memcpy(&msg.header, data.data(), sizeof(MessageHeader));
        
        // Extract payload
        if (data.size() > sizeof(MessageHeader)) {
            msg.payload = data.substr(sizeof(MessageHeader));
        }
        
        return msg;
    }
};

// Helper function to set token
inline void setToken(MessageHeader& header, const std::string& token) {
    size_t copySize = (token.size() < 32) ? token.size() : 32;
    std::memcpy(header.token, token.c_str(), copySize);
}

// Helper function to get token as string
inline std::string getToken(const MessageHeader& header) {
    return std::string(reinterpret_cast<const char*>(header.token), 32);
}
