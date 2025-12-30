#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../components/text_input.h"
#include <SFML/Graphics.hpp>

class LoginState : public GameState {
public:
    explicit LoginState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void onLoginClicked();
    void onSignupClicked();
    void consumeNetworkEvents();

    StateContext& ctx_;
    TextInput usernameInput_;
    TextInput passwordInput_;
    TextInput displayNameInput_;
    Button loginButton_;
    Button signupButton_;
    sf::Text titleText_;
    sf::Text statusText_;
    sf::RectangleShape background_;
    bool showDisplayName_;
};
