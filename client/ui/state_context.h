#pragma once

#include "game_state.h"
#include "../core/network_client.h"
#include "../core/thread_safe_queue.h"
#include "../core/network_event.h"
#include "../handlers/session/client_session.h"
#include <memory>
#include <functional>

enum class GameStateType {
    Login,
    Lobby,
    PrivateChat,
    InGame
};

using StateTransitionCallback = std::function<void(GameStateType)>;

class StateContext {
public:
    NetworkClient& network;
    ClientSession& session;
    ThreadSafeQueue<NetworkEvent>& eventQueue;
    StateTransitionCallback requestTransition;
    sf::Font& font;

    StateContext(NetworkClient& net, ClientSession& sess, ThreadSafeQueue<NetworkEvent>& eq, sf::Font& f)
        : network(net), session(sess), eventQueue(eq), font(f) {}
};
