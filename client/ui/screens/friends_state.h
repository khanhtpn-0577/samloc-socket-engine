#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include "../../core/network_event.h"
#include "../../handlers/friend/friend_handler.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

struct DisplayFriend {
    uint32_t userId;
    std::string username;
    double balance;
    sf::RectangleShape removeButton;
};

struct DisplayPendingRequest {
    uint32_t senderId;
    std::string senderUsername;
    std::string senderDisplayName;
    std::string timestamp;
    sf::RectangleShape acceptButton;
    sf::RectangleShape declineButton;
};

class FriendsState : public GameState {
public:
    explicit FriendsState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    void onBackClicked();
    void onSendFriendRequestClicked();
    void onRemoveFriendClicked(size_t friendIndex);
    void onAcceptRequestClicked(size_t requestIndex);
    void onDeclineRequestClicked(size_t requestIndex);
    void handleNetworkEvent(const NetworkEvent& event);

    // UI Helpers
    void redrawFriendList();
    void redrawPendingRequests();
    void displayMessage(const std::string& text, const std::string& color);
    void showRemovalConfirmation(const std::string& friendUsername);
    void closeRemovalConfirmation();

    StateContext& ctx_;

    // Layout dimensions
    float leftPanelWidth = 0.65f;
    float rightPanelWidth = 0.35f;
    float messageAreaHeight = 60.0f;
    float inputBarHeight = 100.0f;

    // Background and text
    sf::RectangleShape background_;
    sf::Text titleText_;
    sf::Text messageText_;
    sf::Color messageColor_;
    float messageDisplayTime_ = 0.0f;
    const float MESSAGE_DISPLAY_DURATION = 5.0f;

    // Left panel: Friend list
    sf::RectangleShape leftPanel_;
    sf::Text friendListTitleText_;
    std::vector<DisplayFriend> displayFriends_;
    int selectedFriendForRemoval_ = -1;

    // Right panel: Input + Pending requests
    sf::RectangleShape rightPanel_;

    // Input bar
    sf::RectangleShape inputBar_;
    sf::Text inputBarTitleText_;
    sf::RectangleShape usernameInputBox_;
    sf::Text usernameInputText_;
    std::string usernameInput_;
    Button sendButton_;
    const size_t MAX_USERNAME_LENGTH = 30;

    // Pending requests area
    sf::RectangleShape pendingRequestsArea_;
    sf::Text pendingRequestsTitleText_;
    std::vector<DisplayPendingRequest> displayRequests_;

    // Buttons
    Button backButton_;

    // Confirmation dialog
    bool showConfirmationDialog_ = false;
    std::string confirmationFriendName_;
    sf::RectangleShape confirmDialogBackground_;
    sf::Text confirmDialogText_;
    Button confirmYesButton_;
    Button confirmNoButton_;

    // Friend handler
    FriendHandler* friendHandler_ = nullptr;
};
