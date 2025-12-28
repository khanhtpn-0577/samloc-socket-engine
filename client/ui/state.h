#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "../core/network_event.h"

class StateStack;

struct AppContext {
    sf::Font* font;
    sf::RenderWindow* window;
    StateStack* stack;
};

class State {
public:
    State(AppContext ctx) : ctx_(ctx) {}
    virtual ~State() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update(float dt) = 0;
    virtual void draw() = 0;
    virtual void handleNetworkEvent(const NetworkEvent& ev) = 0;

protected:
    AppContext ctx_;
};
