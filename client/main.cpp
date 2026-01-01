#include <SFML/Graphics.hpp>
#include <iostream>

#include "ui/game_manager.h"
#include "ui/state_context.h"
#include "core/network_client.h"
#include "core/thread_safe_queue.h"
#include "core/network_event.h"
#include "handlers/session/client_session.h"

// ===============================
// ENTRY POINT
// ===============================
int main() {
    // -------------------------------
    // Create window
    // -------------------------------
    sf::RenderWindow window(
        sf::VideoMode(1280, 720),
        "Samloc - Private Chat UI Test"
    );
    window.setFramerateLimit(60);

    // -------------------------------
    // Load font
    // -------------------------------
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
        std::cerr << "Failed to load font. Using default (may not render).\n";
        // Continue anyway; SFML will use a default font
    }

    // -------------------------------
    // Mock core components
    // -------------------------------
    ThreadSafeQueue<NetworkEvent> eventQueue;

    ClientSession session;
    session.setLoggedIn(true);
    session.setUserId(1);
    session.setUsername("TestUser");

    // Dummy network config (KHÔNG start)
    NetworkConfig netCfg;
    netCfg.serverIp = "127.0.0.1";
    netCfg.serverPort = 5000;

    NetworkClient network(netCfg, eventQueue);
    //Không gọi network.start()

    // -------------------------------
    // Create StateContext
    // -------------------------------
    StateContext ctx(
        network,
        session,
        eventQueue,
        font
    );

    // -------------------------------
    // Game manager (boot PRIVATE CHAT)
    // -------------------------------
    GameManager gameManager(ctx);

    // -------------------------------
    // Main loop
    // -------------------------------
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            sf::Vector2f mousePos =
                window.mapPixelToCoords(
                    sf::Mouse::getPosition(window)
                );

            gameManager.handleEvent(event, mousePos);
        }

        float dt = clock.restart().asSeconds();
        gameManager.update(dt);

        window.clear(sf::Color(20, 20, 20));
        gameManager.draw(window);
        window.display();
    }

    return 0;
}