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

    // ===== Network =====
    ThreadSafeQueue<NetworkEvent> eventQueue;

    NetworkConfig cfg{
        .serverIp = "127.0.0.1",
        .serverPort = 5000
    };

    NetworkClient network(cfg, eventQueue);
    if (!network.start()) {
        std::cerr << "Failed to start network client\n";
        return 1;
    }

    // ===== Session =====
    ClientSession session;
    session.setUserId(1);
    session.setUsername("Player1");
    session.setLoggedIn(true);

    MessageSender& chatSender = network.chatSender();
    chatSender.updateIdentity(session.userId(), "");
    ChatLogic chatLogic(chatSender);
    ChatHandler chatHandler(chatLogic, session);

    AuthHandler authHandler(session);
    ChallengeHandler challengeHandler(session);
    ClientConnectionHandler connHandler(
        chatHandler,
        authHandler,
        challengeHandler
    );


    // ===== State Context =====
    StateContext ctx(
        network,
        session,
        chatHandler,
        eventQueue,
        font
    );

    // ===== Game Manager =====
    GameManager gameManager(ctx);

    // ⚠️ Bypass Login/Lobby
    gameManager.transitionTo(GameStateType::PrivateChat);

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

        // Xử lý sự kiện mạng
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
