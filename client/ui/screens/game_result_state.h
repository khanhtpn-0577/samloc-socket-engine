#pragma once
#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <vector>

class GameResultState : public GameState {
public:
    explicit GameResultState(StateContext& ctx);
    ~GameResultState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    StateContext& ctx_;
    sf::RectangleShape background_;
    sf::RectangleShape panelBg_;
    sf::Text titleText_;

    struct PlayerRow {
        sf::RectangleShape bar;
        sf::Text name;
        sf::Text status;
        sf::Text penaltyDetail;
        sf::Text totalAmount;
    };

    std::vector<PlayerRow> rows_;
    Button btnBack_;

    void setupLayout();
    sf::String toSfString(const std::string& str);
    std::string formatMoney(double value);
};