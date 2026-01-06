#pragma once

#include "../../net/protocol.h"
#include "../session/client_session.h"

class ChatHandler;
class AuthHandler;
class ChallengeHandler;
class RankHandler;
class FriendHandler;
class LuckyWheelHandler;

class ClientConnectionHandler {
public:
    ClientConnectionHandler(ChatHandler& chatHandler, AuthHandler& authHandler, 
                           ChallengeHandler& challengeHandler, RankHandler& rankHandler, FriendHandler& friendHandler, LuckyWheelHandler& luckyWheelHandler);

    // gọi khi client nhận được message từ server
    void handleMessage(const Message& message);

private:
    ChatHandler& chatHandler_;
    AuthHandler& authHandler_;
    ChallengeHandler& challengeHandler_;
    RankHandler& rankHandler_;
    FriendHandler& friendHandler_;
    LuckyWheelHandler& luckyWheelHandler_;
};
