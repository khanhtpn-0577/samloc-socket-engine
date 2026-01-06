#include "game_manager.h"
#include <iostream>
#include "screens/private_chat_state.h"
#include "screens/friends_state.h"



// GameManager::GameManager(StateContext& ctx)
//     : ctx_(ctx), currentStateType_(GameStateType::Login) {
    
//     // Setup transition callback
//     ctx_.requestTransition = [this](GameStateType newState) {
//         transitionTo(newState);
//     };
    
//     // Create initial state
//     stateCache_[GameStateType::Login] = std::make_unique<LoginState>(ctx_);
//     stateCache_[GameStateType::Lobby] = std::make_unique<LobbyState>(ctx_);
    
//     currentState_ = std::make_unique<LoginState>(ctx_);
//     currentState_->onEnter();
// }

GameManager::GameManager(StateContext& ctx)
    : ctx_(ctx),
      currentStateType_(GameStateType::Login) {

    ctx_.requestTransition = [this](GameStateType newState) {
        std::cout << "[GameManager] Transition requested from "
                  << static_cast<int>(currentStateType_)
                  << " to " << static_cast<int>(newState) << "\n";
        transitionTo(newState);
    };

    std::cout << "[GameManager] Initializing Login state\n";
    currentState_ = std::make_unique<LoginState>(ctx_);
    currentState_->onEnter();
}

void GameManager::transitionTo(GameStateType newState) {
    if (currentStateType_ == newState) {
        std::cout << "[GameManager] Ignoring transition to same state "
                  << static_cast<int>(newState) << "\n";
        return;
    }

    std::cout << "[GameManager] Transitioning from "
              << static_cast<int>(currentStateType_)
              << " to " << static_cast<int>(newState) << "\n";

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
        case GameStateType::PrivateChat:
            currentState_ = std::make_unique<PrivateChatState>(ctx_);
            break;
        case GameStateType::Ranking:
            currentState_ = std::make_unique<RankingState>(ctx_);
            break;
        case GameStateType::Friends:
            currentState_ = std::make_unique<FriendsState>(ctx_);
            break;
        case GameStateType::InGame:
            // TODO: implement InGameState
            std::cerr << "[GameManager] InGame state not implemented yet\n";
            break;
    }

    if (!currentState_) {
        std::cerr << "[GameManager] Failed to create state "
                  << static_cast<int>(newState) << "\n";
        return;
    }

    currentState_->onEnter();
    std::cout << "[GameManager] Entered state "
              << static_cast<int>(newState) << "\n";
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
