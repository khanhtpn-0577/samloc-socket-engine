#include "login_state.h"
#include "../../handlers/auth/auth_handler.h"
#include "../../logic/auth/auth_logic.h"
#include <iostream>
#include <cstdint>

LoginState::LoginState(StateContext& ctx)
    : ctx_(ctx), showDisplayName_(false), pendingSignupUsername_(), pendingSignupPassword_() {
    
    // Title
    titleText_.setFont(ctx_.font);
    titleText_.setString("Samloc - Login");
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0)); // Gold
    titleText_.setPosition(480.f, 80.f);

    // Status
    statusText_.setFont(ctx_.font);
    statusText_.setCharacterSize(18);
    statusText_.setFillColor(sf::Color::Red);
    statusText_.setPosition(400.f, 500.f);

    // Username input
    usernameInput_.setFont(ctx_.font);
    usernameInput_.setPlaceholder("Username");
    usernameInput_.setPosition(sf::Vector2f(440.f, 200.f));
    usernameInput_.setSize(sf::Vector2f(400.f, 40.f));
    usernameInput_.setColors(sf::Color(50, 50, 50), sf::Color::White, sf::Color::White);

    // Password input
    passwordInput_.setFont(ctx_.font);
    passwordInput_.setPlaceholder("Password");
    passwordInput_.setPosition(sf::Vector2f(440.f, 260.f));
    passwordInput_.setSize(sf::Vector2f(400.f, 40.f));
    passwordInput_.setColors(sf::Color(50, 50, 50), sf::Color::White, sf::Color::White);
    passwordInput_.setPasswordMode(true);

    // Display name input (initially hidden)
    displayNameInput_.setFont(ctx_.font);
    displayNameInput_.setPlaceholder("Display Name");
    displayNameInput_.setPosition(sf::Vector2f(440.f, 320.f));
    displayNameInput_.setSize(sf::Vector2f(400.f, 40.f));
    displayNameInput_.setColors(sf::Color(50, 50, 50), sf::Color::White, sf::Color::White);

    // Login button
    loginButton_.setFont(ctx_.font);
    loginButton_.setText("Login", 24);
    loginButton_.setSize(sf::Vector2f(190.f, 50.f));
    loginButton_.setPosition(sf::Vector2f(440.f, 400.f));
    loginButton_.setColors(sf::Color(0, 100, 0), sf::Color::White, sf::Color::White);
    loginButton_.setCallback([this]() { onLoginClicked(); });

    // Signup button
    signupButton_.setFont(ctx_.font);
    signupButton_.setText("Signup", 24);
    signupButton_.setSize(sf::Vector2f(190.f, 50.f));
    signupButton_.setPosition(sf::Vector2f(650.f, 400.f));
    signupButton_.setColors(sf::Color(0, 0, 100), sf::Color::White, sf::Color::White);
    signupButton_.setCallback([this]() { onSignupClicked(); });

    // Background
    background_.setSize(sf::Vector2f(1280.f, 720.f));
    background_.setFillColor(sf::Color(7, 99, 36)); // Dark green felt
}

void LoginState::onEnter() {
    std::cout << "[LoginState] Entered\n";
    usernameInput_.clear();
    passwordInput_.clear();
    displayNameInput_.clear();
    statusText_.setString("");
    showDisplayName_ = false;
    pendingSignupUsername_.clear();
    pendingSignupPassword_.clear();
}

void LoginState::onExit() {
    std::cout << "[LoginState] Exited\n";
}

void LoginState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    usernameInput_.handleEvent(event, mousePos);
    passwordInput_.handleEvent(event, mousePos);
    if (showDisplayName_) {
        displayNameInput_.handleEvent(event, mousePos);
    }
    loginButton_.handleEvent(event, mousePos);
    signupButton_.handleEvent(event, mousePos);
}

void LoginState::update(float dt) {
    (void)dt;
}

void LoginState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);
    usernameInput_.draw(window);
    passwordInput_.draw(window);
    if (showDisplayName_) {
        displayNameInput_.draw(window);
    }
    loginButton_.draw(window);
    signupButton_.draw(window);
    window.draw(statusText_);
}

void LoginState::onSignupClicked() {
    std::string username = usernameInput_.value();
    std::string password = passwordInput_.value();
    std::string displayName = displayNameInput_.value();

    if (username.empty() || password.empty()) {
        statusText_.setString("Username and password required");
        statusText_.setFillColor(sf::Color::Red);
        return;
    }

    // Signup lần 1 → hiện display name
    if (!showDisplayName_) {
        showDisplayName_ = true;
        statusText_.setString("Enter display name and click Signup again");
        statusText_.setFillColor(sf::Color::White);
        return;
    }

    // Signup lần 2
    if (displayName.empty()) {
        displayName = username;
    }

    pendingSignupUsername_ = username;
    pendingSignupPassword_ = password;

    statusText_.setString("Signing up...");
    statusText_.setFillColor(sf::Color::White);

    // CALLBACK
    ctx_.auth_handler.setSignupCallback(
        [this](bool success,
               uint32_t userId,
               const std::string&,
               const std::string&,
               const std::string& message) {

            showDisplayName_ = false;

            if (success) {
                std::cout << "[LoginState] Signup success. Transition to Lobby\n";
                //ctx_.requestTransition(GameStateType::Lobby);
                statusText_.setString("Signup successful! You can now login.");
            } else {
                statusText_.setString("Signup failed: " + message);
                statusText_.setFillColor(sf::Color::Red);
            }
        }
    );

    // Gửi signup
    ctx_.auth_handler.onSignupSender(username, password, displayName);
}


void LoginState::onLoginClicked() {
    std::string username = usernameInput_.value();
    std::string password = passwordInput_.value();

    if (username.empty() || password.empty()) {
        statusText_.setString("Username and password required");
        statusText_.setFillColor(sf::Color::Red);
        return;
    }

    statusText_.setString("Logging in...");
    statusText_.setFillColor(sf::Color::White);

    ctx_.auth_handler.setLoginCallback(
        [this, username](bool success,
                         uint32_t userId,
                         const std::string& token,
                         const std::string& message) {

            if (!success) {
                statusText_.setString("Login failed: " + message);
                statusText_.setFillColor(sf::Color::Red);
                return;
            }

            // Optional nhưng nên có
            ctx_.session.setUsername(username);

            std::cout << "[LoginState] Login success. Transition to Lobby\n";
            ctx_.requestTransition(GameStateType::Lobby);
        }
    );

    // Gửi login request
    ctx_.auth_handler.onLoginSender(username, password);
}
