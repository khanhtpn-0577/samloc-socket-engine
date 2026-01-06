// room_sender.h (FIX – KHÔNG SPAM JOIN, GỬI ĐÚNG GÓI)

#pragma once
#include "../../net/client_socket.h"
#include "../../net/protocol.h"
#include "../session/client_session.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>  // Thêm dòng này
#include <string>    // Nên có nếu dùng std::string hoặc std::to_string

class RoomSender {
public:
    RoomSender(ClientSocket& socket, ClientSession& session)
        : socket_(socket), session_(session) {}

    // ===== LOBBY =====
    bool sendGetRoomList() {
        Message msg;
        msg.header.messageType = (uint16_t)MessageType::C_GET_ROOM_LIST;
        msg.header.senderId = session_.userId();
        msg.payload.clear();
        msg.header.payloadLength = 0;
        setToken(msg.header, session_.token());
        return socket_.sendMessage(msg);
    }

    // ===== JOIN ROOM (CHỈ GỬI 1 LẦN) =====
bool sendJoinRoom(int roomId) {
    // 1. Kiểm tra đầu vào
    if (roomId <= 0) {
        std::cout << "[DEBUG][JoinRoom] Error: Invalid roomId = " << roomId << std::endl;
        return false;
    }

    Message msg;
    msg.header.messageType = (uint16_t)MessageType::C_JOIN_ROOM;
    msg.header.senderId = session_.userId();
    
    // 2. Kiểm tra Payload JSON
    msg.payload = "{\"roomId\":" + std::to_string(roomId) + "}";
    msg.header.payloadLength = msg.payload.size();
    
    setToken(msg.header, session_.token());

    // 3. In toàn bộ thông tin gói tin trước khi gửi
    std::cout << "-------------------------------------------" << std::endl;
    std::cout << "[DEBUG][JoinRoom] Attempting to send message..." << std::endl;
    std::cout << "   + MessageType: " << (int)msg.header.messageType << std::endl;
    std::cout << "   + SenderID:    " << msg.header.senderId << std::endl;
    std::cout << "   + Payload:     " << msg.payload << std::endl;
    std::cout << "   + Length:      " << msg.header.payloadLength << std::endl;
    std::cout << "   + Token:       " << (session_.token().empty() ? "EMPTY" : "EXISTS") << std::endl;

    // 4. Kiểm tra kết quả gửi socket
    bool success = socket_.sendMessage(msg);
    
    if (success) {
        std::cout << "[DEBUG][JoinRoom] SUCCESS: Message sent to socket." << std::endl;
    } else {
        std::cerr << "[DEBUG][JoinRoom] FAILED: Could not write to socket!" << std::endl;
    }
    std::cout << "-------------------------------------------" << std::endl;

    return success;
}
    // ===== READY =====
    bool sendReady(bool ready) {
        Message msg;
        msg.header.messageType = (uint16_t)MessageType::C_READY;
        msg.header.senderId = session_.userId();
        msg.payload = std::string("{\"ready\":") + (ready ? "true" : "false") + "}";
        msg.header.payloadLength = msg.payload.size();
        setToken(msg.header, session_.token());
        return socket_.sendMessage(msg);
    }

    // ===== LEAVE =====
    bool sendLeaveRoom() {
        Message msg;
        msg.header.messageType = (uint16_t)MessageType::C_LEAVE_ROOM;
        msg.header.senderId = session_.userId();
        msg.payload.clear();
        msg.header.payloadLength = 0;
        setToken(msg.header, session_.token());
        return socket_.sendMessage(msg);
    }

    bool sendPlayCards(const std::vector<int>& cardIds) {
        Message msg;
        msg.header.messageType = (uint16_t)MessageType::C_PLAY_CARD;
        msg.header.senderId = session_.userId();

        std::stringstream ss;
        ss << "{\"cards\":[";
        for (size_t i = 0; i < cardIds.size(); ++i) {
            ss << cardIds[i];
            if (i + 1 < cardIds.size()) ss << ",";
        }
        ss << "]}";

        msg.payload = ss.str();
        msg.header.payloadLength = msg.payload.size();
        setToken(msg.header, session_.token());
        return socket_.sendMessage(msg);
    }

    bool sendPassTurn() {
        Message msg;
        msg.header.messageType = (uint16_t)MessageType::C_PASS_TURN;
        msg.header.senderId = session_.userId();
        msg.payload = "{}";
        msg.header.payloadLength = msg.payload.size();
        setToken(msg.header, session_.token());
        return socket_.sendMessage(msg);
    }

private:
    ClientSocket& socket_;
    ClientSession& session_;
};
