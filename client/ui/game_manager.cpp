#include "game_manager.h"
#include <iostream>

GameManager::GameManager(StateContext& ctx)
    : ctx_(ctx), currentStateType_(GameStateType::Login) {
    
    // Setup transition callback
    ctx_.requestTransition = [this](GameStateType newState) {
        transitionTo(newState);
    };
    
    // Create initial state
    stateCache_[GameStateType::Login] = std::make_unique<LoginState>(ctx_);
    stateCache_[GameStateType::Lobby] = std::make_unique<LobbyState>(ctx_);
    
    currentState_ = std::make_unique<LoginState>(ctx_);
    currentState_->onEnter();
}

void GameManager::transitionTo(GameStateType newState) {
    if (currentStateType_ == newState) {
        return;
    }
    
    std::cout << "[GameManager] Transition to state " << static_cast<int>(newState) << "\n";
    
    if (currentState_) {
        currentState_->onExit();
    }
    
    currentStateType_ = newState;
    
    switch (newState) {
        case GameStateType::Login:
            currentState_ = std::make_unique<LoginState>(ctx_);
            break;
        case GameStateType::Lobby:
            currentState_ = std::make_unique<LobbyState>(ctx_);
            break;
        case GameStateType::InGame:
            // TODO: implement InGameState
            std::cerr << "[GameManager] InGame state not implemented yet\n";
            break;
    }
    
    if (currentState_) {
        currentState_->onEnter();
    }
}

void GameManager::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (currentState_) {
        currentState_->handleEvent(event, mousePos);
    }
}

void GameManager::update(float dt) {
    if (currentState_) {
        currentState_->update(dt);
    }
}

void GameManager::draw(sf::RenderWindow& window) {
    if (currentState_) {
        currentState_->draw(window);
    }
}
