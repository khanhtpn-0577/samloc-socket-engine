#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class LuckyWheelState : public GameState {
public:
    explicit LuckyWheelState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event,
                     const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    StateContext& ctx_;

    // ===== Background =====
    sf::RectangleShape background_;

    // ===== Title =====
    sf::Text titleText_;

    // ===== Balance Info =====
    sf::Text balanceText_;

    // ===== Wheel =====
    sf::Vector2f wheelCenter_;
    std::vector<sf::ConvexShape> wheelSectors_;
    sf::CircleShape outerCircle_;
    sf::CircleShape centerCircle_;
    float currentRotation_;
    float rotationSpeed_;
    bool spinning_;

    // ===== Network Integration =====
    bool waitingForServer_;      // Đang chờ server trả kết quả
    int targetRewardIndex_;      // Index của phần thưởng từ server
    float targetRotation_;       // Góc quay đích để dừng đúng reward
    bool hasServerResult_;       // Đã nhận được kết quả từ server
    float slowdownTimer_;        // Thời gian quay chậm sau khi nhận kết quả (2s)
    long long pendingNewBalance_;   // balance mới từ server, chờ animation kết thúc


    // ===== Rewards =====
    std::vector<std::string> rewards_;
    std::vector<sf::Text> rewardTexts_;

    // ===== UI =====
    Button spinButton_;
    Button backButton_;

private:
    void initRewards();
};