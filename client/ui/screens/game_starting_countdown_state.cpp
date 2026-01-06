#include "game_starting_countdown_state.h"
#include <iostream>

// helper
sf::String GameStartingCountdownState::toSfString(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

GameStartingCountdownState::GameStartingCountdownState(StateContext& ctx)
    : ctx_(ctx), totalSeconds_(0), remainingSeconds_(0) {

    background_.setSize({1280,720});
    background_.setFillColor(sf::Color(0,0,0,180));

    countdownText_.setFont(ctx_.font);
    countdownText_.setCharacterSize(140);
    countdownText_.setFillColor(sf::Color(255,215,0));
    countdownText_.setStyle(sf::Text::Bold);

    messageText_.setFont(ctx_.font);
    messageText_.setString(toSfString("CHUẨN BỊ VÀO VÁN"));
    messageText_.setCharacterSize(28);
    messageText_.setFillColor(sf::Color::White);

    sf::FloatRect mRect = messageText_.getLocalBounds();
    messageText_.setOrigin(mRect.left + mRect.width/2.f, mRect.top + mRect.height/2.f);
    messageText_.setPosition(640.f, 260.f);
}

void GameStartingCountdownState::onEnter() {
    remainingSeconds_ = 0;
    totalSeconds_ = 0;
    countdownText_.setString("");

    // receive countdown seconds from server
    ctx_.roomHandler.setGameCountdownCallback([this](int seconds) {
        startCountdown(seconds);
    });

    // receive game start (hand) and transition
    ctx_.roomHandler.setGameStartCallback([this](const std::vector<int>& hand) {
        ctx_.myHand = hand;
        ctx_.requestTransition(GameStateType::InGame);
    });
}

void GameStartingCountdownState::onExit() {
    ctx_.roomHandler.setGameCountdownCallback(nullptr);
    ctx_.roomHandler.setGameStartCallback(nullptr);
}

void GameStartingCountdownState::startCountdown(int seconds) {
    totalSeconds_ = remainingSeconds_ = seconds;
    startTime_ = std::chrono::steady_clock::now();

    // initial set
    countdownText_.setString(std::to_string(remainingSeconds_));
    sf::FloatRect cRect = countdownText_.getLocalBounds();
    countdownText_.setOrigin(cRect.left + cRect.width/2.f, cRect.top + cRect.height/2.f);
    countdownText_.setPosition(640.f, 380.f);
}

void GameStartingCountdownState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    (void)event;
    (void)mousePos;
}

void GameStartingCountdownState::update(float dt) {
    (void)dt;
    if (totalSeconds_ <= 0) return;
    int elapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - startTime_).count();

    int newRemain = totalSeconds_ - elapsed;
    if (newRemain < 0) newRemain = 0;

    if (newRemain != remainingSeconds_) {
        remainingSeconds_ = newRemain;
        countdownText_.setString(std::to_string(remainingSeconds_));
        sf::FloatRect cRect = countdownText_.getLocalBounds();
        countdownText_.setOrigin(cRect.left + cRect.width/2.f, cRect.top + cRect.height/2.f);
        countdownText_.setPosition(640.f, 380.f);
    }
}

void GameStartingCountdownState::draw(sf::RenderWindow& w) {
    w.draw(background_);
    w.draw(messageText_);
    w.draw(countdownText_);
}
