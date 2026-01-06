// client_connection_handler.cpp
#include "client_connection_handler.h"
#include "../chat/chat_handler.h"
#include "../auth/auth_handler.h"
#include "../challenge/challenge_handler.h"
#include "../rank/rank_handler.h"
#include "../room/room_handler.h"
#include <iostream>

ClientConnectionHandler::ClientConnectionHandler(
    ChatHandler& chatHandler,
    AuthHandler& authHandler,
    ChallengeHandler& challengeHandler,
    RankHandler& rankHandler,
    RoomHandler& roomHandler
):
    chatHandler_(chatHandler),
    authHandler_(authHandler),
    challengeHandler_(challengeHandler),
    rankHandler_(rankHandler),
    roomHandler_(roomHandler)
{}

void ClientConnectionHandler::handleMessage(const Message& message) {
    MessageType type = static_cast<MessageType>(message.header.messageType);

    switch (type) {
        // CHAT HANDLER MESSAGES
        case MessageType::CHAT_DIRECT:
        case MessageType::CHAT_ROOM:
        case MessageType::SERVER_DELIVER_MESSAGE: // legacy / alias if defined elsewhere
            chatHandler_.onServerDeliverMessage(message);
            break;

        case MessageType::CHAT_DIRECT_ACK:
            chatHandler_.onServerACK(message);
            break;

        case MessageType::FRIEND_LIST_RESPONSE:
            chatHandler_.onServerDeliverFriendList(message);
            break;

        case MessageType::PRIVATE_CHAT_HISTORY_RESPONSE:
            chatHandler_.onServerDeliverPrivateChatHistory(message);
            break;

        // AUTH HANDLER MESSAGES
        case MessageType::SIGNUP_RESPONSE:
            authHandler_.onSignupResponse(message);
            break;
        case MessageType::LOGIN_RESPONSE:
            authHandler_.onLoginResponse(message);
            break;
        case MessageType::LOGOUT_RESPONSE:
            authHandler_.onLogoutResponse(message);
            break;

        // CHALLENGE HANDLER MESSAGES
        case MessageType::CHALLENGE_NOTIFICATION:
            challengeHandler_.onChallengeNotification(message);
            break;
        case MessageType::SEND_CHALLENGE_RESPONSE:
            challengeHandler_.onSendChallengeResponse(message);
            break;
        case MessageType::ACCEPT_CHALLENGE_RESPONSE:
            challengeHandler_.onAcceptChallengeResponse(message);
            break;
        case MessageType::REJECT_CHALLENGE_RESPONSE:
            challengeHandler_.onRejectChallengeResponse(message);
            break;
        case MessageType::CANCEL_CHALLENGE_RESPONSE:
            challengeHandler_.onCancelChallengeResponse(message);
            break;
        case MessageType::CHALLENGE_EXPIRED:
            challengeHandler_.onChallengeExpired(message);
            break;

        // RANK HANDLER MESSAGES
        case MessageType::FRIEND_RANK_RESPONSE:
            rankHandler_.onFriendRankResponse(message);
            break;

        // ROOM & GAME FLOW MESSAGES
        case MessageType::S_ROOM_LIST:
            roomHandler_.onRoomListReceived(message);
            break;

        case MessageType::S_JOIN_ROOM_FAIL:
            roomHandler_.onJoinRoomResult(message, false);
            break;

        case MessageType::S_PLAYER_JOINED:
            // light ack - let join callback know success
            roomHandler_.onJoinRoomResult(message, true);
            break;

        case MessageType::S_EXISTING_PLAYERS:
            // When receiving existing players we consider join success + room update
            roomHandler_.onJoinRoomResult(message, true);
            roomHandler_.onRoomUpdateReceived(message);
            break;

        case MessageType::S_PLAYER_LEFT:
        case MessageType::S_PLAYER_READY:
        case MessageType::S_ROOM_UPDATE:
            roomHandler_.onRoomUpdateReceived(message);
            break;

        case MessageType::S_GAME_START_COUNTDOWN:
            roomHandler_.onGameCountdownReceived(message);
            break;

        case MessageType::S_GAME_START:
            roomHandler_.onGameStartReceived(message);
            break;

        case MessageType::S_MOVE_RESULT:
            roomHandler_.onMoveResultReceived(message);
            break;

        case MessageType::PLAYER_DISCONNECT_GAME:
            std::cerr << "[ClientConn] Unhandled Game/Error message type: 0x"
                      << std::hex << (int)message.header.messageType << std::dec << "\n";
            // TODO: route to in-game state if required
            break;

        // CLIENT -> SERVER messages: these are sent by client, not expected here
        case MessageType::C_GET_ROOM_LIST:
        case MessageType::C_JOIN_ROOM:
        case MessageType::C_READY:
        case MessageType::C_PLAY_CARD:
        case MessageType::C_PASS_TURN:
        case MessageType::SIGNUP:
        case MessageType::LOGIN:
        case MessageType::LOGOUT:
        case MessageType::SEND_CHALLENGE:
        case MessageType::ACCEPT_CHALLENGE:
        case MessageType::REJECT_CHALLENGE:
        case MessageType::CANCEL_CHALLENGE:
        case MessageType::FRIEND_LIST_REQUEST:
        case MessageType::PRIVATE_CHAT_HISTORY_REQUEST:
        case MessageType::FRIEND_RANK_REQUEST:
            // These are client-originated; nothing to do on client inbound path.
            break;

        default:
            std::cerr << "[ClientConnectionHandler] Unknown message type: 0x"
                      << std::hex << (int)message.header.messageType << std::dec << "\n";
            break;
    }
}
