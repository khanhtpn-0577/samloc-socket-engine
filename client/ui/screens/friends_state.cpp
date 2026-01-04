#include "friends_state.h"
#include "lobby_state.h"
#include "../../handlers/friend/friend_handler.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

FriendsState::FriendsState(StateContext& ctx)
    : ctx_(ctx) {
    std::cout << "[FriendsState] Constructed\n";
}

void FriendsState::onEnter() {
    std::cout << "[FriendsState] Entering Friends screen\n";

    // Get window size
    float windowWidth = 1280.0f;
    float windowHeight = 720.0f;

    std::cout << "[FriendsState] Window size: " << windowWidth << "x" << windowHeight << "\n";

    // Setup handler from context
    friendHandler_ = &ctx_.friendHandler;
    std::cout << "[FriendsState] Friend handler initialized\n";

    // Setup callbacks
    friendHandler_->setMessageCallback([this](const std::string& msg, const std::string& color) {
        displayMessage(msg, color);
    });

    friendHandler_->setPendingRequestsCallback([this](const auto& requests) {
        displayRequests_.clear();
        for (const auto& req : requests) {
            DisplayPendingRequest displayReq;
            displayReq.senderId = req.first;
            displayReq.senderUsername = req.second.first;
            displayReq.senderDisplayName = req.second.second.first;
            displayReq.timestamp = req.second.second.second;
            displayRequests_.push_back(displayReq);
        }
        redrawPendingRequests();
        std::cout << "[FriendsState] Pending requests updated: " << displayRequests_.size() << "\n";
    });

    friendHandler_->setFriendListCallback([this](const auto& friends) {
        displayFriends_.clear();
        for (const auto& f : friends) {
            DisplayFriend displayFriend;
            displayFriend.userId = f.first;
            displayFriend.username = f.second.first;
            displayFriend.balance = f.second.second;
            displayFriends_.push_back(displayFriend);
        }
        redrawFriendList();
        std::cout << "[FriendsState] Friend list updated: " << displayFriends_.size() << " friends\n";
    });

    // Background
    background_.setSize(sf::Vector2f(windowWidth, windowHeight));
    background_.setFillColor(sf::Color(40, 40, 40));

    // Title
    titleText_.setFont(ctx_.font);
    titleText_.setString("My friend list");
    titleText_.setPosition(20.0f, 10.0f);
    titleText_.setCharacterSize(28);
    titleText_.setFillColor(sf::Color::White);

    // Left panel (65%)
    float leftWidth = windowWidth * leftPanelWidth;
    leftPanel_.setSize(sf::Vector2f(leftWidth - 10.0f, windowHeight - 50.0f));
    leftPanel_.setPosition(10.0f, 50.0f);
    leftPanel_.setFillColor(sf::Color(50, 50, 50));

    friendListTitleText_.setFont(ctx_.font);
    friendListTitleText_.setString("Friends");
    friendListTitleText_.setPosition(20.0f, 60.0f);
    friendListTitleText_.setCharacterSize(18);
    friendListTitleText_.setFillColor(sf::Color::Yellow);

    // Right panel (35%)
    float rightX = leftWidth + 10.0f;
    float rightWidth = windowWidth - rightX - 10.0f;
    rightPanel_.setSize(sf::Vector2f(rightWidth, windowHeight - 50.0f));
    rightPanel_.setPosition(rightX, 50.0f);
    rightPanel_.setFillColor(sf::Color(50, 50, 50));

    // Input bar (top 30% of right panel)
    float inputBarY = 60.0f;
    inputBar_.setSize(sf::Vector2f(rightWidth - 20.0f, 90.0f));
    inputBar_.setPosition(rightX + 10.0f, inputBarY);
    inputBar_.setFillColor(sf::Color(60, 60, 60));

    inputBarTitleText_.setFont(ctx_.font);
    inputBarTitleText_.setString("Send a friend request");
    inputBarTitleText_.setPosition(rightX + 15.0f, inputBarY + 5.0f);
    inputBarTitleText_.setCharacterSize(14);
    inputBarTitleText_.setFillColor(sf::Color::White);

    usernameInputBox_.setSize(sf::Vector2f(rightWidth - 120.0f, 35.0f));
    usernameInputBox_.setPosition(rightX + 15.0f, inputBarY + 30.0f);
    usernameInputBox_.setFillColor(sf::Color(70, 70, 70));
    usernameInputBox_.setOutlineColor(sf::Color::White);
    usernameInputBox_.setOutlineThickness(1.0f);

    usernameInputText_.setFont(ctx_.font);
    usernameInputText_.setCharacterSize(14);
    usernameInputText_.setFillColor(sf::Color::White);
    usernameInputText_.setPosition(rightX + 20.0f, inputBarY + 35.0f);

    // Setup send button
    sendButton_.setFont(ctx_.font);
    sendButton_.setText("Send", 18);
    sendButton_.setSize(sf::Vector2f(90.0f, 35.0f));
    sendButton_.setPosition(sf::Vector2f(rightX + rightWidth - 105.0f, inputBarY + 30.0f));
    sendButton_.setColors(sf::Color(0, 100, 160), sf::Color::White, sf::Color::White);
    sendButton_.setCallback([this]() { onSendFriendRequestClicked(); });

    // Message area (3 lines at top)
    messageText_.setFont(ctx_.font);
    messageText_.setCharacterSize(14);
    messageText_.setFillColor(sf::Color::White);
    messageText_.setPosition(rightX + 15.0f, 55.0f);
    messageColor_ = sf::Color::White;

    // Pending requests area (bottom 70% of right panel)
    float pendingY = inputBarY + 100.0f;
    pendingRequestsArea_.setSize(sf::Vector2f(rightWidth - 20.0f, windowHeight - pendingY - 60.0f));
    pendingRequestsArea_.setPosition(rightX + 10.0f, pendingY);
    pendingRequestsArea_.setFillColor(sf::Color(60, 60, 60));

    pendingRequestsTitleText_.setFont(ctx_.font);
    pendingRequestsTitleText_.setString("Pending friend requests");
    pendingRequestsTitleText_.setPosition(rightX + 15.0f, pendingY + 5.0f);
    pendingRequestsTitleText_.setCharacterSize(14);
    pendingRequestsTitleText_.setFillColor(sf::Color::Yellow);

    // Back button
    backButton_.setFont(ctx_.font);
    backButton_.setText("< Back", 18);
    backButton_.setSize(sf::Vector2f(100.0f, 35.0f));
    backButton_.setPosition(sf::Vector2f(windowWidth - 120.0f, windowHeight - 50.0f));
    backButton_.setColors(sf::Color(120, 0, 0), sf::Color::White, sf::Color::White);
    backButton_.setCallback([this]() { ctx_.requestTransition(GameStateType::Lobby); });

    // Confirmation dialog
    confirmDialogBackground_.setSize(sf::Vector2f(400.0f, 150.0f));
    confirmDialogBackground_.setFillColor(sf::Color(30, 30, 30));
    confirmDialogBackground_.setOutlineColor(sf::Color::White);
    confirmDialogBackground_.setOutlineThickness(2.0f);

    confirmDialogText_.setFont(ctx_.font);
    confirmDialogText_.setCharacterSize(16);
    confirmDialogText_.setFillColor(sf::Color::White);

    // Setup confirm buttons
    confirmYesButton_.setFont(ctx_.font);
    confirmYesButton_.setText("Yes", 16);
    confirmYesButton_.setSize(sf::Vector2f(80.0f, 35.0f));
    confirmYesButton_.setPosition(sf::Vector2f(150.0f, 240.0f));
    confirmYesButton_.setColors(sf::Color(0, 120, 0), sf::Color::White, sf::Color::White);

    confirmNoButton_.setFont(ctx_.font);
    confirmNoButton_.setText("No", 16);
    confirmNoButton_.setSize(sf::Vector2f(80.0f, 35.0f));
    confirmNoButton_.setPosition(sf::Vector2f(250.0f, 240.0f));
    confirmNoButton_.setColors(sf::Color(120, 0, 0), sf::Color::White, sf::Color::White);

    // Request pending requests from server
    uint32_t userId = ctx_.session.userId();
    std::cout << "[FriendsState] Requesting pending requests for userId=" << userId << "\n";
    // Will be initialized by state machine
    if (friendHandler_) {
        friendHandler_->onRequestPendingRequests(userId);
        friendHandler_->onRequestFriendList(userId);
    }

    displayMessage("Friend list loaded", "green");
}

void FriendsState::onExit() {
    std::cout << "[FriendsState] Exiting Friends screen\n";
}

void FriendsState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    std::cout << "[FriendsState] Handling event\n";

    // Handle confirmation dialog
    if (showConfirmationDialog_) {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (confirmYesButton_.isClicked(mousePos)) {
                std::cout << "[FriendsState] Confirmed friend removal\n";
                if (selectedFriendForRemoval_ >= 0 && selectedFriendForRemoval_ < static_cast<int>(displayFriends_.size())) {
                    uint32_t friendId = displayFriends_[selectedFriendForRemoval_].userId;
                    std::cout << "[FriendsState] Removing friend: " << friendId << "\n";
                    if (friendHandler_) {
                        friendHandler_->onRemoveFriendClicked(ctx_.session.userId(), friendId);
                    }
                }
                closeRemovalConfirmation();
            } else if (confirmNoButton_.isClicked(mousePos)) {
                std::cout << "[FriendsState] Cancelled friend removal\n";
                closeRemovalConfirmation();
            }
        }
        return;
    }

    // Handle button events
    backButton_.handleEvent(event, mousePos);
    sendButton_.handleEvent(event, mousePos);

    if (event.type == sf::Event::MouseButtonPressed) {

        // Handle friend list remove buttons
        for (size_t i = 0; i < displayFriends_.size(); i++) {
            if (displayFriends_[i].removeButton.getGlobalBounds().contains(mousePos)) {
                std::cout << "[FriendsState] Remove friend button clicked for index " << i << "\n";
                onRemoveFriendClicked(i);
                return;
            }
        }

        // Handle pending request accept/decline buttons
        for (size_t i = 0; i < displayRequests_.size(); i++) {
            if (displayRequests_[i].acceptButton.getGlobalBounds().contains(mousePos)) {
                std::cout << "[FriendsState] Accept request button clicked for index " << i << "\n";
                onAcceptRequestClicked(i);
                return;
            }
            if (displayRequests_[i].declineButton.getGlobalBounds().contains(mousePos)) {
                std::cout << "[FriendsState] Decline request button clicked for index " << i << "\n";
                onDeclineRequestClicked(i);
                return;
            }
        }
    }

    // Handle text input
    if (event.type == sf::Event::TextEntered) {
        if (usernameInput_.length() < MAX_USERNAME_LENGTH) {
            char c = static_cast<char>(event.text.unicode);
            // Allow only alphanumeric characters
            if (std::isalnum(c) || c == '_') {
                usernameInput_ += c;
                usernameInputText_.setString(usernameInput_);
                std::cout << "[FriendsState] Username input: " << usernameInput_ << "\n";
            }
        }
    }

    // Handle backspace
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::BackSpace && !usernameInput_.empty()) {
            usernameInput_.pop_back();
            usernameInputText_.setString(usernameInput_);
            std::cout << "[FriendsState] Username input (after backspace): " << usernameInput_ << "\n";
        }
    }
}

void FriendsState::update(float dt) {
    messageDisplayTime_ -= dt;
}

void FriendsState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);

    // Draw left panel
    window.draw(leftPanel_);
    window.draw(friendListTitleText_);

    // Draw friend list items
    float friendItemY = 90.0f;
    for (const auto& friendInfo : displayFriends_) {
        // Friend info text
        std::stringstream ss;
        ss << friendInfo.username << " - Balance: " << std::fixed << std::setprecision(0) << friendInfo.balance;
        sf::Text friendText(ss.str(), ctx_.font, 12);
        friendText.setPosition(30.0f, friendItemY);
        friendText.setFillColor(sf::Color::White);
        window.draw(friendText);

        // Draw remove button
        window.draw(friendInfo.removeButton);
        sf::Text xText("X", ctx_.font, 14);
        xText.setFillColor(sf::Color::White);
        float xPos = friendInfo.removeButton.getPosition().x + 6.0f;
        float yPos = friendInfo.removeButton.getPosition().y + 3.0f;
        xText.setPosition(xPos, yPos);
        window.draw(xText);

        friendItemY += 35.0f;
    }

    // Draw right panel
    window.draw(rightPanel_);

    // Draw message area
    if (messageDisplayTime_ > 0) {
        window.draw(messageText_);
    }

    // Draw input bar
    window.draw(inputBar_);
    window.draw(inputBarTitleText_);
    window.draw(usernameInputBox_);
    window.draw(usernameInputText_);
    sendButton_.draw(window);

    // Draw pending requests area
    window.draw(pendingRequestsArea_);
    window.draw(pendingRequestsTitleText_);

    float requestItemY = pendingRequestsArea_.getPosition().y + 35.0f;
    for (const auto& req : displayRequests_) {
        // Request info
        std::stringstream ss;
        ss << req.senderDisplayName << " (" << req.senderUsername << ") " << req.timestamp;
        sf::Text requestText(ss.str(), ctx_.font, 11);
        requestText.setPosition(pendingRequestsArea_.getPosition().x + 15.0f, requestItemY);
        requestText.setFillColor(sf::Color::White);
        window.draw(requestText);

        // Draw accept/decline buttons
        window.draw(req.acceptButton);
        window.draw(req.declineButton);

        sf::Text acceptText("✓", ctx_.font, 14);
        acceptText.setFillColor(sf::Color::Green);
        acceptText.setPosition(req.acceptButton.getPosition().x + 5.0f, req.acceptButton.getPosition().y + 2.0f);
        window.draw(acceptText);

        sf::Text declineText("✕", ctx_.font, 14);
        declineText.setFillColor(sf::Color::Red);
        declineText.setPosition(req.declineButton.getPosition().x + 6.0f, req.declineButton.getPosition().y + 2.0f);
        window.draw(declineText);

        requestItemY += 35.0f;
    }

    // Draw back button
    backButton_.draw(window);

    // Draw confirmation dialog if active
    if (showConfirmationDialog_) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
        overlay.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(overlay);

        float dialogX = (window.getSize().x - 400.0f) / 2.0f;
        float dialogY = (window.getSize().y - 150.0f) / 2.0f;
        confirmDialogBackground_.setPosition(dialogX, dialogY);
        window.draw(confirmDialogBackground_);

        confirmDialogText_.setPosition(dialogX + 20.0f, dialogY + 30.0f);
        window.draw(confirmDialogText_);

        confirmYesButton_.setPosition(sf::Vector2f(dialogX + 70.0f, dialogY + 100.0f));
        confirmNoButton_.setPosition(sf::Vector2f(dialogX + 190.0f, dialogY + 100.0f));
        confirmYesButton_.draw(window);
        confirmNoButton_.draw(window);
    }
}

void FriendsState::onSendFriendRequestClicked() {
    std::cout << "[FriendsState] Send friend request clicked\n";

    if (usernameInput_.empty()) {
        displayMessage("Enter username", "red");
        return;
    }

    std::cout << "[FriendsState] Sending friend request to: " << usernameInput_ << "\n";
    if (friendHandler_) {
        friendHandler_->onSendFriendRequestClicked(ctx_.session.userId(), usernameInput_);
    }
    usernameInput_ = "";
    usernameInputText_.setString("");
}

void FriendsState::onRemoveFriendClicked(size_t friendIndex) {
    std::cout << "[FriendsState] Remove friend clicked for index " << friendIndex << "\n";

    if (friendIndex < displayFriends_.size()) {
        selectedFriendForRemoval_ = friendIndex;
        showRemovalConfirmation(displayFriends_[friendIndex].username);
    }
}

void FriendsState::onAcceptRequestClicked(size_t requestIndex) {
    std::cout << "[FriendsState] Accept request clicked for index " << requestIndex << "\n";

    if (requestIndex < displayRequests_.size()) {
        uint32_t senderId = displayRequests_[requestIndex].senderId;
        std::cout << "[FriendsState] Accepting request from senderId=" << senderId << "\n";
        if (friendHandler_) {
            friendHandler_->onAcceptRequestClicked(ctx_.session.userId(), senderId);
        }
    }
}

void FriendsState::onDeclineRequestClicked(size_t requestIndex) {
    std::cout << "[FriendsState] Decline request clicked for index " << requestIndex << "\n";

    if (requestIndex < displayRequests_.size()) {
        uint32_t senderId = displayRequests_[requestIndex].senderId;
        std::cout << "[FriendsState] Declining request from senderId=" << senderId << "\n";
        if (friendHandler_) {
            friendHandler_->onDeclineRequestClicked(ctx_.session.userId(), senderId);
        }
    }
}

void FriendsState::displayMessage(const std::string& text, const std::string& color) {
    std::cout << "[FriendsState] Displaying message: " << text << " (color=" << color << ")\n";

    messageText_.setString(text);
    if (color == "green") {
        messageColor_ = sf::Color::Green;
    } else if (color == "red") {
        messageColor_ = sf::Color::Red;
    } else if (color == "yellow") {
        messageColor_ = sf::Color::Yellow;
    } else {
        messageColor_ = sf::Color::White;
    }
    messageText_.setFillColor(messageColor_);
    messageDisplayTime_ = MESSAGE_DISPLAY_DURATION;
}

void FriendsState::redrawFriendList() {
    std::cout << "[FriendsState] Redrawing friend list\n";

    float leftWidth = 1280.0f * leftPanelWidth;
    float friendY = 90.0f;

    for (auto& friendInfo : displayFriends_) {
        // Position remove button
        friendInfo.removeButton.setSize(sf::Vector2f(25.0f, 25.0f));
        friendInfo.removeButton.setPosition(leftWidth - 50.0f, friendY + 5.0f);
        friendInfo.removeButton.setFillColor(sf::Color::Red);

        friendY += 35.0f;
    }

    std::cout << "[FriendsState] Friend list redrawn: " << displayFriends_.size() << " friends\n";
}

void FriendsState::redrawPendingRequests() {
    std::cout << "[FriendsState] Redrawing pending requests\n";

    float windowWidth = 1280.0f;
    float rightX = windowWidth * leftPanelWidth + 10.0f;
    float rightWidth = windowWidth - rightX - 10.0f;
    float pendingY = 160.0f + 100.0f;  // After input bar
    float requestY = pendingY + 35.0f;

    for (auto& req : displayRequests_) {
        // Position accept button
        req.acceptButton.setSize(sf::Vector2f(30.0f, 25.0f));
        req.acceptButton.setPosition(rightX + rightWidth - 70.0f, requestY + 5.0f);
        req.acceptButton.setFillColor(sf::Color::Green);

        // Position decline button
        req.declineButton.setSize(sf::Vector2f(30.0f, 25.0f));
        req.declineButton.setPosition(rightX + rightWidth - 35.0f, requestY + 5.0f);
        req.declineButton.setFillColor(sf::Color::Red);

        requestY += 35.0f;
    }

    std::cout << "[FriendsState] Pending requests redrawn: " << displayRequests_.size() << " requests\n";
}

void FriendsState::showRemovalConfirmation(const std::string& friendUsername) {
    std::cout << "[FriendsState] Showing removal confirmation for: " << friendUsername << "\n";

    showConfirmationDialog_ = true;
    confirmationFriendName_ = friendUsername;
    confirmDialogText_.setString("Are you sure you want to unfriend " + friendUsername + "?");
}

void FriendsState::closeRemovalConfirmation() {
    std::cout << "[FriendsState] Closing removal confirmation\n";

    showConfirmationDialog_ = false;
    selectedFriendForRemoval_ = -1;
    confirmationFriendName_ = "";
}
