#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

struct PlayerItem {
    uint32_t userId;
    std::string username;
    sf::Text nameText;
    Button challengeButton;
};

struct PendingChallenge {
    uint32_t challengeId;
    uint32_t senderId;
    uint32_t receiverId;
    std::string senderName;
    sf::Text infoText;
    Button acceptButton;
    Button rejectButton;
};

class LobbyState : public GameState {
public:
    explicit LobbyState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void consumeNetworkEvents();
    void onLogoutClicked();
    void rebuildPlayerList();
    void rebuildChallengeList();

    StateContext& ctx_;
    sf::RectangleShape background_;
    sf::Text titleText_;
    sf::Text sessionText_;
    Button logoutButton_;
    
    std::vector<PlayerItem> playerList_;
    std::vector<PendingChallenge> pendingChallenges_;
};
