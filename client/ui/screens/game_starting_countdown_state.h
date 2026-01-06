#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include <SFML/Graphics.hpp>
#include <chrono>

class GameStartingCountdownState : public GameState {
public:
    explicit GameStartingCountdownState(StateContext& ctx);
    
    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void startCountdown(int seconds); // Server gửi về để bắt đầu đếm ngược

private:
    StateContext& ctx_;
    sf::RectangleShape background_;
    sf::Text countdownText_;
    sf::Text messageText_;

    std::chrono::steady_clock::time_point startTime_;
    int totalSeconds_;
    int remainingSeconds_;

    // Helpers
    sf::String toSfString(const std::string& str);
};

