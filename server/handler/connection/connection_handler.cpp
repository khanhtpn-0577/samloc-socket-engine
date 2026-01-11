#include "connection_handler.h"
#include "../chat/chat_handler.h"
#include "../auth/auth_handler.h"
#include "../rank/rank_handler.h"
#include "../challenge/challenge_handler.h"
#include "../friend/friend_handler.h"
#include "../game/game_handler.h"
#include "../private_room/private_room_handler.h"
#include "../../logic/lucky_wheel/lucky_wheel_logic.h"
#include "../../handler/lucky_wheel/lucky_wheel_handler.h"
#include "../../logic/chat/chat_logic.h"
#include "../../logic/auth/auth_logic.h"
#include "../../logic/challenge/challenge_logic.h"
#include "../../logic/rank/rank_logic.h"
#include "../../logic/friend/friend_logic.h"
#include "../../logic/private_room/private_room_logic.h"
#include "../session/session_manager.h"
#include "../../db/database.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstring>

static std::string toHex(const uint8_t* data, size_t len) {
    std::stringstream ss;
    ss << std::hex;
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
    }
    return ss.str();
}

ConnectionHandler::ConnectionHandler(int fd, Database& dbRef)
    : clientFd(fd), boundUserId(0), db(dbRef) {
    inputBuffer.reserve(4096);
}

bool ConnectionHandler::sendAll(const char* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        #ifdef MSG_NOSIGNAL
            ssize_t s = send(clientFd, data + sent, size - sent, MSG_NOSIGNAL);
        #else
            ssize_t s = send(clientFd, data + sent, size - sent, 0);
        #endif

        if (s < 0) {
            if (errno == EINTR) continue;
            std::cerr << "[SEND ERROR] fd=" << clientFd
                      << " errno=" << errno
                      << " (" << strerror(errno) << ")\n";
            return false;
        }
        if (s == 0) return false;
        sent += static_cast<size_t>(s);
    }
    return true;
}


bool ConnectionHandler::sendMessage(const Message& msg) {
    Message safeMsg = msg;
    safeMsg.header.payloadLength = static_cast<uint32_t>(safeMsg.payload.size());
    std::string bytes = safeMsg.serialize();
    bool ok = sendAll(bytes.data(), bytes.size());
    std::cout
        << "[SEND Message]"
        << " fd=" << clientFd
        << " bytes=" << bytes.size()
        << " type=0x" << std::hex << safeMsg.header.messageType
        << " (" << std::dec << safeMsg.header.messageType << ")"
        << " payloadLen=" << safeMsg.header.payloadLength
        << " payload=";
    return ok;
}

bool ConnectionHandler::onReadable() {
    char temp[4096];
    ssize_t n = recv(clientFd, temp, sizeof(temp), 0);
    if (n > 0) {
        inputBuffer.insert(inputBuffer.end(), temp, temp + n);
        size_t showLen = std::min<size_t>(50, inputBuffer.size());
        std::cout << "[DEBUG HEX] fd=" << clientFd << " bytes=" << inputBuffer.size() << " data=" << toHex(reinterpret_cast<const uint8_t*>(inputBuffer.data()), showLen) << (inputBuffer.size() > showLen ? " ..." : "") << "\n";
    } else if (n == 0) {
        std::cout << "[DISCONNECT] fd=" << clientFd << "\n";
        return false;
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return true;
        std::cerr << "[RECV ERROR] fd=" << clientFd << " errno=" << errno << " (" << strerror(errno) << ")\n";
        return false;
    }

    while (true) {
        if (inputBuffer.size() < sizeof(MessageHeader)) break;
        MessageHeader header;
        std::memcpy(&header, inputBuffer.data(), sizeof(MessageHeader));
        constexpr uint32_t MAX_PAYLOAD = 1024 * 1024;
        if (header.payloadLength > MAX_PAYLOAD) {
            std::cerr << "[INVALID PAYLOAD] too large: " << header.payloadLength << "\n";
            return false;
        }
        size_t totalSize = sizeof(MessageHeader) + header.payloadLength;
        if (inputBuffer.size() < totalSize) break;
        Message msg;
        msg.header = header;
        if (header.payloadLength > 0) {
            msg.payload.assign(inputBuffer.begin() + sizeof(MessageHeader), inputBuffer.begin() + totalSize);
        }
        std::cout << "[PARSE] fd=" << clientFd << " type=0x" << std::hex << msg.header.messageType << std::dec << " sender=" << msg.header.senderId << " payload_len=" << msg.header.payloadLength << "\n";
        processIncomingMessage(msg);
        inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + totalSize);
    }

    return true;
}

void ConnectionHandler::processIncomingMessage(const Message& incoming) {
    if (boundUserId == 0 && incoming.header.senderId != 0) {
        boundUserId = incoming.header.senderId;
        SessionManager::instance().add(boundUserId, this);
        std::cout << "[BIND USER] uid=" << boundUserId << " fd=" << clientFd << "\n";
    }

    GameHandler::instance().init(db);
    ChatLogic chatLogic;
    AuthLogic authLogic(db);
    ChallengeLogic challengeLogic(db);
    RankLogic rankLogic(db);
    FriendLogic friendLogic(db);
    LuckyWheelLogic luckyWheelLogic(db);
    PrivateRoomLogic privateRoomLogic(db);

    ChatHandler chatHandler(chatLogic);
    FriendHandler friendHandler(friendLogic);
    AuthHandler authHandler(authLogic);
    ChallengeHandler challengeHandler(challengeLogic);
    RankHandler rankHandler(rankLogic);
    PrivateRoomHandler privateRoomHandler(privateRoomLogic);
    
    
    LuckyWheelHandler luckyWheelHandler(luckyWheelLogic);

    Message response;
    bool needRespond = false;
    uint16_t typeVal = incoming.header.messageType;

    if (typeVal >= 0x2000 && typeVal <= 0x30FF) {
        response = GameHandler::instance().handleMessage(incoming, incoming.header.senderId);
        if (response.header.messageType != 0) needRespond = true;
    } else {
        switch (static_cast<MessageType>(typeVal)) {
            case MessageType::CHAT_DIRECT:
                response = chatHandler.handleChatDirect(incoming);
                needRespond = true;
                break;
            
            case MessageType::FRIEND_LIST_REQUEST:
                std::cout <<"[Server] Handling FRIEND_LIST_REQUEST\n";
                response = chatHandler.handleFriendListRequest(incoming);
                needRespond = true;
                break;

            case MessageType::PRIVATE_CHAT_HISTORY_REQUEST:
                std::cout <<"[Server] Handling PRIVATE_CHAT_HISTORY_REQUEST\n";
                response = chatHandler.handlePrivateChatHistoryRequest(incoming);
                needRespond = true;
                break;
            case MessageType::SIGNUP:
                response = authHandler.handleSignup(incoming);
                needRespond = true;
                break;
            case MessageType::LOGIN:
                response = authHandler.handleLogin(incoming);
                needRespond = true;
                break;
            case MessageType::LOGOUT:
                response = authHandler.handleLogout(incoming);
                needRespond = true;
                break;
            case MessageType::REQUEST_BALANCE:
                response = authHandler.handleRequestBalance(incoming);
                std::cout <<"[Connection handler] Handling REQUEST_BALANCE\n";
                needRespond = true;
                break;
            case MessageType::SEND_CHALLENGE:
                response = challengeHandler.handleSendChallenge(incoming);
                std::cout <<"[Connection handler] Handling SEND_CHALLENGE\n";
                needRespond = true;
                break;
            
            case MessageType::FRIEND_RANK_REQUEST:
                std::cout <<"[Connection handler] Handling FRIEND_RANK_REQUEST\n";
                response = rankHandler.handleFriendRankRequest(incoming);
                needRespond = true;
                break;
            case MessageType::SEND_FRIEND_REQUEST:
                response = friendHandler.handleSendFriendRequest(incoming);
                needRespond = true;
                break;
            case MessageType::ACCEPT_FRIEND_REQUEST:
                response = friendHandler.handleAcceptFriendRequest(incoming);
                needRespond = true;
                break;
            case MessageType::DECLINE_FRIEND_REQUEST:
                response = friendHandler.handleDeclineFriendRequest(incoming);
                needRespond = true;
                break;
            case MessageType::REMOVE_FRIEND:
                response = friendHandler.handleRemoveFriend(incoming);
                needRespond = true;
                break;
            case MessageType::GET_PENDING_REQUESTS:
                response = friendHandler.handleGetPendingRequests(incoming);
                needRespond = true;
                break;
            case MessageType::GET_FRIEND_LIST:
                response = friendHandler.handleGetFriendList(incoming);
                needRespond = true;
                break;
            case MessageType::LUCKY_WHEEL_SPIN_REQUEST:
                std::cout <<"[Connection handler] Handling LUCKY_WHEEL_SPIN_REQUEST\n";
                response = luckyWheelHandler.handleSpinRequest(incoming);
                needRespond = true;
                break;
            case MessageType::C_CREATE_PRIVATE_ROOM:
                std::cout <<"[Connection handler] Handling C_CREATE_PRIVATE_ROOM\n";
                response = privateRoomHandler.handleCreatePrivateRoom(incoming);
                needRespond = true;
                break;
            default:
                std::cerr << "[UNKNOWN TYPE] " << typeVal << "\n";
                break;
        }
    }

    if (needRespond) {
        if (!sendMessage(response)) {
            std::cerr << "[SEND FAIL] fd=" << clientFd << "\n";
        }
        std::cout << "[Connection handler] Sent response of type "
                  << response.header.messageType
                  << " to fd=" << clientFd << " with payload: " << response.payload << "\n";
    }
}

void ConnectionHandler::closeConnection() {
    if (boundUserId != 0) {
        GameHandler::instance().onClientDisconnect(boundUserId);
        SessionManager::instance().remove(boundUserId);
        std::cout << "[USER OFFLINE] uid=" << boundUserId << "\n";
    }
    close(clientFd);
}
