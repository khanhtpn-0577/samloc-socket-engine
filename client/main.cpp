#include <SFML/Graphics.hpp>
#include <iostream>

#include "ui/game_manager.h"
#include "ui/state_context.h"
#include "ui/screens/private_chat_state.h"

#include "core/network_client.h"
#include "core/thread_safe_queue.h"
#include "core/network_event.h"

#include "handlers/session/client_session.h"

#include "handlers/connection/client_connection_handler.h"
#include "handlers/auth/auth_handler.h"
#include "handlers/challenge/challenge_handler.h"
#include "handlers/friend/friend_handler.h"

#include "logic/auth/auth_logic.h"
#include "logic/chat/chat_logic.h"
#include "logic/friend/friend_logic.h"


// Hàm xử lý các sự kiện den client
void consumeNetworkEvents(
    ThreadSafeQueue<NetworkEvent>& eventQueue,
    ClientConnectionHandler& connHandler
) {
    while (true) {
        auto opt = eventQueue.tryPop();
        if (!opt.has_value()) break;
        NetworkEvent& ev = *opt;

        if (std::holds_alternative<RawMessageEvent>(ev.payload)) {
            const Message& msg =
                std::get<RawMessageEvent>(ev.payload).message;
            connHandler.handleMessage(msg);
        }
        else if (std::holds_alternative<DisconnectEvent>(ev.payload)) {
            const auto& dc =
                std::get<DisconnectEvent>(ev.payload);
            std::cerr << "[Client] Disconnected: "
                      << dc.reason << "\n";
        }
    }
}


int main(){
    // ===== Window =====
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Samloc - Private Chat");
    window.setFramerateLimit(60);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        std::cerr << "Failed to load font. Using default (may not render).\n";
        // Continue anyway; SFML will use a default font
    }

    // ===== Session =====
    ClientSession session;

    // ===== Network =====
    ThreadSafeQueue<NetworkEvent> eventQueue;

    NetworkConfig cfg{
        .serverIp = "127.0.0.1",
        .serverPort = 5000
    };

    NetworkClient network(cfg, eventQueue, session);
    if (!network.start()) {
        std::cerr << "Failed to start network client\n";
        return 1;
    }

    MessageSender& chatSender = network.chatSender();
    ChatLogic chatLogic(chatSender);
    ChatHandler chatHandler(chatLogic, session);

    AuthSender& authSender = network.authSender();
    AuthLogic authLogic(authSender);
    AuthHandler authHandler(authLogic, session);
    
    FriendSender& friendSender = network.friendSender();
    FriendLogic friendLogic(friendSender);
    FriendHandler friendHandler(friendLogic);
    
    ChallengeHandler challengeHandler(session);

    RankSender& rankSender = network.rankSender();
    RankLogic rankLogic(rankSender);
    RankHandler rankHandler(rankLogic, session);

    
    ClientConnectionHandler connHandler(
        chatHandler,
        authHandler,
        challengeHandler,
        rankHandler,
        friendHandler
    );


    // ===== State Context =====
    StateContext ctx(
        network,
        session,
        chatHandler,
        rankHandler,
        eventQueue,
        authHandler,
        friendHandler,
        font
    );

    // ===== Game Manager =====
    GameManager gameManager(ctx);

    // Start with Login/Signup screen
    gameManager.transitionTo(GameStateType::Login);

    // ===== Main Loop =====
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        sf::Vector2f mousePos =
            window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            gameManager.handleEvent(event, mousePos);
        }

        // Process network events
        consumeNetworkEvents(eventQueue, connHandler);

        float dt = clock.restart().asSeconds();
        gameManager.update(dt);

        window.clear();
        gameManager.draw(window);
        window.display();
    }

    // ===== Cleanup =====
    network.stop();
    return 0;
}
