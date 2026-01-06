#pragma once

#include "../../core/network_client.h"
class ChatHandler;
class AuthHandler;
class ChallengeHandler;
class RankHandler;
class RoomHandler;
class FriendHandler;
class LuckyWheelHandler;

class ClientConnectionHandler {
public:
    ClientConnectionHandler(ChatHandler& chatHandler, AuthHandler& authHandler, 
                           ChallengeHandler& challengeHandler, RankHandler& rankHandler, FriendHandler& friendHandler, LuckyWheelHandler& luckyWheelHandler, RoomHandler& roomHandler);

    void handleMessage(const Message& message);

private:
    ChatHandler& chatHandler_;
    AuthHandler& authHandler_;
    ChallengeHandler& challengeHandler_;
    RankHandler& rankHandler_;
    RoomHandler& roomHandler_;
    FriendHandler& friendHandler_;
    LuckyWheelHandler& luckyWheelHandler_;
};
