#include "login_state.h"
#include "../../handlers/auth/auth_handler.h"
#include <iostream>

LoginState::LoginState(StateContext& ctx)
    : ctx_(ctx), showDisplayName_(false) {
    
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
    consumeNetworkEvents();
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

void LoginState::onLoginClicked() {
    std::string username = usernameInput_.value();
    std::string password = passwordInput_.value();

    if (username.empty() || password.empty()) {
        statusText_.setString("Username and password required");
        return;
    }

    statusText_.setString("Logging in...");
    ctx_.network.authSender().sendLogin(username, password);
}

void LoginState::onSignupClicked() {
    std::string username = usernameInput_.value();
    std::string password = passwordInput_.value();
    std::string displayName = displayNameInput_.value();

    if (username.empty() || password.empty()) {
        statusText_.setString("Username and password required");
        return;
    }

    if (!showDisplayName_) {
        showDisplayName_ = true;
        statusText_.setString("Enter display name and click Signup again");
        return;
    }

    if (displayName.empty()) {
        displayName = username;
    }

    statusText_.setString("Signing up...");
    ctx_.network.authSender().sendSignup(username, password, displayName);
}

void LoginState::consumeNetworkEvents() {
    AuthHandler authHandler(ctx_.session);

    auto parseField = [](const std::string& payload, const std::string& key) {
        std::string searchKey = "\"" + key + "\":\"";
        size_t keyPos = payload.find(searchKey);
        if (keyPos == std::string::npos) return std::string();
        size_t valueStart = keyPos + searchKey.length();
        size_t valueEnd = payload.find("\"", valueStart);
        if (valueEnd == std::string::npos) return std::string();
        return payload.substr(valueStart, valueEnd - valueStart);
    };

    while (auto opt = ctx_.eventQueue.tryPop()) {
        NetworkEvent& ev = *opt;

        if (std::holds_alternative<DisconnectEvent>(ev.payload)) {
            statusText_.setString("Disconnected: " + std::get<DisconnectEvent>(ev.payload).reason);
            continue;
        }

        if (std::holds_alternative<RawMessageEvent>(ev.payload)) {
            Message& msg = std::get<RawMessageEvent>(ev.payload).message;
            MessageType type = static_cast<MessageType>(msg.header.messageType);

            if (type == MessageType::SIGNUP_RESPONSE) {
                bool success = msg.payload.find("\"success\":true") != std::string::npos;
                std::string serverMsg = parseField(msg.payload, "message");
                std::string statusMsg = success ? "Signup successful! You can now login." : ("Signup failed: " + serverMsg);
                statusText_.setString(statusMsg);
                statusText_.setFillColor(success ? sf::Color::Green : sf::Color::Red);
                showDisplayName_ = false;
            } else if (type == MessageType::LOGIN_RESPONSE) {
                bool success = msg.payload.find("\"success\":true") != std::string::npos;
                if (success) {
                    authHandler.onLoginResponse(msg);
                    
                    // Update all senders with new identity
                    uint32_t userId = ctx_.session.userId();
                    std::string token = ctx_.session.token();
                    ctx_.network.authSender().updateIdentity(userId, token);
                    ctx_.network.chatSender().updateIdentity(userId, token);
                    ctx_.network.challengeSender().updateIdentity(userId, token);
                    
                    ctx_.requestTransition(GameStateType::Lobby);
                } else {
                    std::string serverMsg = parseField(msg.payload, "message");
                    statusText_.setString("Login failed: " + serverMsg);
                    statusText_.setFillColor(sf::Color::Red);
                }
            }
        }
    }
}
