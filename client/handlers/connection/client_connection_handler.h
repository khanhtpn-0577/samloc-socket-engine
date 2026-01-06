#pragma once

#include "../../core/network_client.h"
class ChatHandler;
class AuthHandler;
class ChallengeHandler;
class RankHandler;
class RoomHandler;

class ClientConnectionHandler {
public:
    ClientConnectionHandler(
        ChatHandler& chatHandler,
        AuthHandler& authHandler,
        ChallengeHandler& challengeHandler,
        RankHandler& rankHandler,
        RoomHandler& roomHandler
    );

    void handleMessage(const Message& message);

private:
    ChatHandler& chatHandler_;
    AuthHandler& authHandler_;
    ChallengeHandler& challengeHandler_;
    RankHandler& rankHandler_;
    RoomHandler& roomHandler_;
};