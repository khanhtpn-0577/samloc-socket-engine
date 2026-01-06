#pragma once

#include "game_state.h"
#include "state_context.h"
#include "screens/login_state.h"
#include "screens/lobby_state.h"
#include "screens/private_chat_state.h"
#include "screens/ranking_state.h"
#include "screens/room_list_state.h" 
#include "screens/waiting_room_state.h" 
#include "screens/game_starting_countdown_state.h" 
#include "screens/in_game_state.h"
#include <memory>
#include <map>

class GameManager {
public:
    GameManager(StateContext& ctx);

    void transitionTo(GameStateType newState);
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    StateContext& ctx_;
    GameStateType currentStateType_;
    std::unique_ptr<GameState> currentState_;
    std::map<GameStateType, std::unique_ptr<GameState>> stateCache_;
};
