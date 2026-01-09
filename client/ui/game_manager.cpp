#include "game_manager.h"
#include <iostream>
#include "screens/private_chat_state.h"
#include "screens/friends_state.h"

#include "screens/login_state.h"
#include "screens/lobby_state.h"
#include "screens/private_chat_state.h"
#include "screens/ranking_state.h"
#include "screens/room_list_state.h"
#include "screens/waiting_room_state.h"
#include "screens/game_starting_countdown_state.h"
#include "screens/in_game_state.h"

GameManager::GameManager(StateContext& ctx)
    : ctx_(ctx),
      currentStateType_(GameStateType::Login) {

    ctx_.requestTransition = [this](GameStateType newState) {
        std::cout << "[GameManager] Transition requested to " << static_cast<int>(newState) << "\n";
        transitionTo(newState);
    };

    currentState_ = std::make_unique<LoginState>(ctx_);
    currentState_->onEnter();
}

void GameManager::transitionTo(GameStateType newState) {
    if (currentStateType_ == newState && currentState_ != nullptr) {
        currentState_->onExit();
        currentState_->onEnter();
        return;
    }

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
        case GameStateType::RoomList:
            currentState_ = std::make_unique<RoomListState>(ctx_);
            break;
        case GameStateType::WaitingRoom:
            currentState_ = std::make_unique<WaitingRoomState>(ctx_);
            break;
        case GameStateType::GameStartingCountdown:
            currentState_ = std::make_unique<GameStartingCountdownState>(ctx_);
            break;
        case GameStateType::Friends:
            currentState_ = std::make_unique<FriendsState>(ctx_);
            break;
        case GameStateType::LuckyWheel:
            currentState_ = std::make_unique<LuckyWheelState>(ctx_);
            break;
        case GameStateType::InGame:
            currentState_ = std::make_unique<InGameState>(ctx_);
            break;
        case GameStateType::GameResult:
            currentState_ = std::make_unique<GameResultState>(ctx_);
            break;
        default:
            currentState_ = nullptr;
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