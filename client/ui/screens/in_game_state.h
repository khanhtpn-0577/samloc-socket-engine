#pragma once
#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../components/card_ui.h" 
#include "../../handlers/room/room_structs.h" 
#include <vector>
#include <SFML/Graphics.hpp>

class InGameState : public GameState {
public:
    explicit InGameState(StateContext& ctx);
    ~InGameState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    void updateOpponentPlayers(const std::vector<RoomMember>& members);
    void updateTurnInfo(int currentPlayerId, int timeout);
    void updatePlayedCards(int userId, const std::vector<int>& cardsPlayed, int cardsLeft, const std::string& action = "play");

private:
    StateContext& ctx_;
    
    sf::RectangleShape background_;
    sf::RectangleShape tableCenterBg_;
    sf::Text roomInfoText_; 
    sf::Text turnInfoText_; 

    // Toast Message
    sf::Text toastText_;
    float toastTimer_ = 0.f;

    // Smooth Timer
    float visualTimeout_ = 0.f;

    std::vector<CardUI> myHand_;
    std::vector<CardUI> playedCardsOnTable_; 
    sf::Vector2f playedCardsOffset_;

    Button btnPlay_;
    Button btnPass_;
    Button btnSort_;
    Button btnLeaveGame_;

    struct OpponentUI {
        sf::RectangleShape panelBg;
        sf::CircleShape avatarBg;
        sf::Text avatarLetter;
        sf::Text nameText;
        sf::Text cardsLeftText; 
        sf::Vector2f position; 

        void setup(sf::Font& font);
        void setContent(const RoomMember& member, int cardsCount, bool isTurn);
        void setEmpty();
        void draw(sf::RenderWindow& window);
    };
    OpponentUI opponentSlots_[3];

    void layoutHand();
    void setupOpponentLayout();
    void onPlayClicked();
    void onPassClicked();
    void onSortClicked();
    void onLeaveGameClicked();
    void showToast(const std::string& msg);
    void syncHandFromServer(); // Đồng bộ lại từ ctx_
};