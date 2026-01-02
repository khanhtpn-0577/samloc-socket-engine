#include "client_connection_handler.h"
#include "../chat/chat_handler.h"
#include "../auth/auth_handler.h"
#include "../challenge/challenge_handler.h"
#include <iostream>

//constructor
ClientConnectionHandler::ClientConnectionHandler(
    ChatHandler& chatHandler,
    AuthHandler& authHandler,
    ChallengeHandler& challengeHandler
):
    chatHandler_(chatHandler),
    authHandler_(authHandler),
    challengeHandler_(challengeHandler){}

void ClientConnectionHandler::handleMessage(const Message& message){
    MessageType type = static_cast<MessageType>(message.header.messageType);
    std::cout << "[ClientConnectionHandler] Received message. "
              << "type=0x"
              << std::hex << message.header.messageType
              << std::dec << "\n";

    switch (type) {
        case MessageType::CHAT_DIRECT_ACK:
            chatHandler_.onServerACK(message);
            break;

        case MessageType::CHAT_DIRECT:
            chatHandler_.onServerDeliverMessage(message);
            break;

        case MessageType::SIGNUP_RESPONSE:
            std::cout << "[ClientConnectionHandler] Received SIGNUP_RESPONSE with payload: " << message.payload << "\n";
            authHandler_.onSignupResponse(message);
            break;

        case MessageType::LOGIN_RESPONSE:
            std::cout << "[ClientConnectionHandler] Received LOGIN_RESPONSE with payload: " << message.payload << "\n";
            authHandler_.onLoginResponse(message);
            break;

        case MessageType::LOGOUT_RESPONSE:
            authHandler_.onLogoutResponse(message);
            break;

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

        case MessageType::FRIEND_LIST_RESPONSE:
            chatHandler_.onServerDeliverFriendList(message);
            break;
        
        case MessageType::PRIVATE_CHAT_HISTORY_RESPONSE:
            chatHandler_.onServerDeliverPrivateChatHistory(message);
            break;

        default:
            std::cerr << "[ClientConnectionHandler] "
                      << "Unhandled message type: "
                      << message.header.messageType << "\n";
            break;
    }
}
