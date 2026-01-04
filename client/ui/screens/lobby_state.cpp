#include "lobby_state.h"
#include <iostream>

LobbyState::LobbyState(StateContext& ctx)
    : ctx_(ctx) {

    // ===== Background =====
    background_.setSize({1280.f, 720.f});
    background_.setFillColor(sf::Color(7, 99, 36));

    // ===== Title =====
    titleText_.setFont(ctx_.font);
    titleText_.setString("Lobby");
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setPosition(40.f, 20.f);

    // ===== Session Info =====
    sessionText_.setFont(ctx_.font);
    sessionText_.setCharacterSize(18);
    sessionText_.setFillColor(sf::Color::White);
    sessionText_.setPosition(40.f, 80.f);

    // ===== Balance Info =====
    balanceText_.setFont(ctx_.font);
    balanceText_.setCharacterSize(20);
    balanceText_.setFillColor(sf::Color(255, 215, 0)); // vàng cho tiền
    balanceText_.setPosition(40.f, 110.f);


    // ===== Logout Button =====
    logoutButton_.setFont(ctx_.font);
    logoutButton_.setText("Logout", 20);
    logoutButton_.setSize({150.f, 40.f});
    logoutButton_.setPosition({1100.f, 30.f});
    logoutButton_.setColors(
        sf::Color(120, 0, 0),
        sf::Color::White,
        sf::Color::White
    );
    logoutButton_.setCallback([this]() { onLogoutClicked(); });

    // ===== Chat Button =====
    chatButton_.setFont(ctx_.font);
    chatButton_.setText("Chat with friends", 22);
    chatButton_.setSize({300.f, 60.f});
    chatButton_.setPosition({490.f, 320.f});
    chatButton_.setColors(
        sf::Color(0, 100, 160),
        sf::Color::White,
        sf::Color::White
    );
    chatButton_.setCallback([this]() { onChatClicked(); });

    // ===== Ranking Button =====
    rankingButton_.setFont(ctx_.font);
    rankingButton_.setText("Friends Ranking", 22);
    rankingButton_.setSize({300.f, 60.f});
    rankingButton_.setPosition({490.f, 400.f});
    rankingButton_.setColors(
        sf::Color(120, 80, 160),
        sf::Color::White,
        sf::Color::White
    );
    rankingButton_.setCallback([this]() {
        ctx_.requestTransition(GameStateType::Ranking);
    });

}

void LobbyState::onEnter() {
    std::cout << "[LobbyState] Entered\n";

    sessionText_.setString(
        "Logged in as: " +
        ctx_.session.username() +
        " (ID: " +
        std::to_string(ctx_.session.userId()) +
        ")"
    );

    balanceText_.setString(
        "Balance: " +
        std::to_string(static_cast<long long>(ctx_.session.balance()))
    );
}

void LobbyState::onExit() {
    std::cout << "[LobbyState] Exited\n";
}

void LobbyState::handleEvent(const sf::Event& event,
                             const sf::Vector2f& mousePos) {
    logoutButton_.handleEvent(event, mousePos);
    chatButton_.handleEvent(event, mousePos);
    rankingButton_.handleEvent(event, mousePos);
}

void LobbyState::update(float dt) {
    (void)dt;
    // Lobby không poll network nữa
}

void LobbyState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);
    window.draw(sessionText_);
    window.draw(balanceText_);

    logoutButton_.draw(window);
    chatButton_.draw(window);
    rankingButton_.draw(window);
}

void LobbyState::onLogoutClicked() {
    std::cout << "[LobbyState] Logout clicked\n";

    ctx_.network.authSender().sendLogout();
    ctx_.session.setLoggedIn(false);
    ctx_.requestTransition(GameStateType::Login);
}

void LobbyState::onChatClicked() {
    std::cout << "[LobbyState] Go to PrivateChatState\n";
    ctx_.requestTransition(GameStateType::PrivateChat);
}
