#pragma once

#include "../../net/protocol.h"
#include "../session/client_session.h"

class ChatHandler;
class AuthHandler;
class ChallengeHandler;

class ClientConnectionHandler {
public:
    ClientConnectionHandler(ChatHandler& chatHandler, AuthHandler& authHandler, 
                           ChallengeHandler& challengeHandler);

    // gọi khi client nhận được message từ server
    void handleMessage(const Message& message);

private:
    ChatHandler& chatHandler_;
    AuthHandler& authHandler_;
    ChallengeHandler& challengeHandler_;
};
