#pragma once

#include "game_state.h"
#include "../core/network_client.h"
#include "../core/thread_safe_queue.h"
#include "../core/network_event.h"
#include "../handlers/session/client_session.h"
#include "../handlers/chat/chat_handler.h"
#include "../handlers/auth/auth_handler.h"
#include "../handlers/rank/rank_handler.h"
#include "../handlers/friend/friend_handler.h"
#include <memory>
#include <functional>

enum class GameStateType {
    Login,
    Lobby,
    PrivateChat,
    InGame,
    Ranking,
    Friends
};

using StateTransitionCallback = std::function<void(GameStateType)>;

class StateContext {
public:
    NetworkClient& network;
    ClientSession& session;
    ChatHandler& chatHandler;
    RankHandler& rankHandler;
    ThreadSafeQueue<NetworkEvent>& eventQueue;
    StateTransitionCallback requestTransition;
    AuthHandler& auth_handler;
    FriendHandler& friendHandler;
    sf::Font& font;

        StateContext(NetworkClient& net,
                                 ClientSession& sess,
                                 ChatHandler& chat_handler,
                                 RankHandler& rank_handler,
                                 ThreadSafeQueue<NetworkEvent>& eq,
                                 AuthHandler& auth_handler,
                                 FriendHandler& friend_handler,
                                 sf::Font& f)
                : network(net),
                    session(sess),
                    chatHandler(chat_handler),
                    rankHandler(rank_handler),
                    eventQueue(eq),
                    requestTransition(nullptr),
                    auth_handler(auth_handler),
                    friendHandler(friend_handler),
                    font(f) {}
};
