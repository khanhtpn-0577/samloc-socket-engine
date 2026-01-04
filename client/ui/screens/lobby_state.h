#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include <SFML/Graphics.hpp>

class LobbyState : public GameState {
public:
    explicit LobbyState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event,
                     const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void onLogoutClicked();
    void onChatClicked();
    void onFriendsClicked();

    StateContext& ctx_;

    sf::RectangleShape background_;
    sf::Text titleText_;
    sf::Text sessionText_;
    sf::Text balanceText_;


    Button logoutButton_;
    Button chatButton_;
    Button rankingButton_;
    Button friendsButton_;
};
