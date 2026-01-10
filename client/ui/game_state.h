#pragma once

#include <SFML/Graphics.hpp>

class GameState {
public:
    virtual ~GameState() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void onResume() {}
    virtual void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};
