#include "lobby_state.h"
#include "../../handlers/challenge/challenge_handler.h"
#include <iostream>

LobbyState::LobbyState(StateContext& ctx)
    : ctx_(ctx) {
    
    // Background
    background_.setSize(sf::Vector2f(1280.f, 720.f));
    background_.setFillColor(sf::Color(7, 99, 36));

    // Title
    titleText_.setFont(ctx_.font);
    titleText_.setString("Lobby");
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setPosition(40.f, 20.f);

    // Session info
    sessionText_.setFont(ctx_.font);
    sessionText_.setCharacterSize(18);
    sessionText_.setFillColor(sf::Color::White);
    sessionText_.setPosition(40.f, 80.f);

    // Logout button
    logoutButton_.setFont(ctx_.font);
    logoutButton_.setText("Logout", 20);
    logoutButton_.setSize(sf::Vector2f(150.f, 40.f));
    logoutButton_.setPosition(sf::Vector2f(1100.f, 30.f));
    logoutButton_.setColors(sf::Color(100, 0, 0), sf::Color::White, sf::Color::White);
    logoutButton_.setCallback([this]() { onLogoutClicked(); });
}

void LobbyState::onEnter() {
    std::cout << "[LobbyState] Entered\n";
    sessionText_.setString("Logged in as: " + ctx_.session.username() + " (ID: " + std::to_string(ctx_.session.userId()) + ")");
    
    // Mock player list for now (replace with real data from server)
    playerList_.clear();
    playerList_.reserve(10); // Reserve space to avoid reallocations
    
    for (uint32_t i = 1; i <= 5; ++i) {
        if (i == ctx_.session.userId()) continue;
        
        PlayerItem item;
        item.userId = i;
        item.username = "Player" + std::to_string(i);
        item.nameText.setFont(ctx_.font);
        item.nameText.setString(item.username);
        item.nameText.setCharacterSize(20);
        item.nameText.setFillColor(sf::Color::White);
        
        item.challengeButton.setFont(ctx_.font);
        item.challengeButton.setText("Challenge", 18);
        item.challengeButton.setSize(sf::Vector2f(140.f, 35.f));
        item.challengeButton.setColors(sf::Color(0, 100, 0), sf::Color::White, sf::Color::White);
        // Don't set callback here - will be set during event handling
        
        playerList_.push_back(std::move(item));
    }
    
    rebuildPlayerList();
}

void LobbyState::onExit() {
    std::cout << "[LobbyState] Exited\n";
}

void LobbyState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    logoutButton_.handleEvent(event, mousePos);
    
    // Handle player challenge buttons with direct click detection
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        for (auto& player : playerList_) {
            if (player.challengeButton.isClicked(mousePos)) {
                ctx_.network.challengeSender().sendChallenge(player.userId);
                std::cout << "Sent challenge to user " << player.userId << "\n";
            }
        }
        
        for (size_t i = 0; i < pendingChallenges_.size(); ++i) {
            auto& challenge = pendingChallenges_[i];
            if (challenge.acceptButton.isClicked(mousePos)) {
                ctx_.network.challengeSender().acceptChallenge(challenge.challengeId);
                pendingChallenges_.erase(pendingChallenges_.begin() + i);
                rebuildChallengeList();
                break;
            }
            if (challenge.rejectButton.isClicked(mousePos)) {
                ctx_.network.challengeSender().rejectChallenge(challenge.challengeId);
                pendingChallenges_.erase(pendingChallenges_.begin() + i);
                rebuildChallengeList();
                break;
            }
        }
    }
}

void LobbyState::update(float dt) {
    (void)dt;
    consumeNetworkEvents();
}

void LobbyState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);
    window.draw(sessionText_);
    logoutButton_.draw(window);
    
    for (const auto& player : playerList_) {
        window.draw(player.nameText);
        player.challengeButton.draw(window);
    }
    
    for (const auto& challenge : pendingChallenges_) {
        window.draw(challenge.infoText);
        challenge.acceptButton.draw(window);
        challenge.rejectButton.draw(window);
    }
}

void LobbyState::onLogoutClicked() {
    ctx_.network.authSender().sendLogout();
    ctx_.session.setLoggedIn(false);
    ctx_.requestTransition(GameStateType::Login);
}

void LobbyState::rebuildPlayerList() {
    float yOffset = 140.f;
    for (auto& player : playerList_) {
        player.nameText.setPosition(60.f, yOffset);
        player.challengeButton.setPosition(sf::Vector2f(300.f, yOffset - 5.f));
        yOffset += 50.f;
    }
}

void LobbyState::rebuildChallengeList() {
    float yOffset = 140.f;
    float xOffset = 600.f;
    
    for (auto& challenge : pendingChallenges_) {
        challenge.infoText.setPosition(xOffset, yOffset);
        challenge.acceptButton.setPosition(sf::Vector2f(xOffset + 250.f, yOffset - 5.f));
        challenge.rejectButton.setPosition(sf::Vector2f(xOffset + 410.f, yOffset - 5.f));
        yOffset += 60.f;
    }
}

void LobbyState::consumeNetworkEvents() {
    ChallengeHandler challengeHandler(ctx_.session);

    while (auto opt = ctx_.eventQueue.tryPop()) {
        NetworkEvent& ev = *opt;

        if (std::holds_alternative<DisconnectEvent>(ev.payload)) {
            std::cout << "Disconnected: " << std::get<DisconnectEvent>(ev.payload).reason << "\n";
            ctx_.requestTransition(GameStateType::Login);
            continue;
        }

        if (std::holds_alternative<RawMessageEvent>(ev.payload)) {
            Message& msg = std::get<RawMessageEvent>(ev.payload).message;
            MessageType type = static_cast<MessageType>(msg.header.messageType);

            if (type == MessageType::CHALLENGE_NOTIFICATION) {
                challengeHandler.onChallengeNotification(msg);
                
                // Parse challenge details
                std::string payload = msg.payload;
                size_t idPos = payload.find("\"challengeId\":");
                size_t senderPos = payload.find("\"senderId\":");
                
                if (idPos != std::string::npos && senderPos != std::string::npos) {
                    uint32_t challengeId = std::stoul(payload.substr(idPos + 14));
                    uint32_t senderId = std::stoul(payload.substr(senderPos + 11));
                    
                    PendingChallenge pc;
                    pc.challengeId = challengeId;
                    pc.senderId = senderId;
                    pc.receiverId = ctx_.session.userId();
                    pc.senderName = "Player" + std::to_string(senderId);
                    
                    pc.infoText.setFont(ctx_.font);
                    pc.infoText.setString("Challenge from " + pc.senderName);
                    pc.infoText.setCharacterSize(20);
                    pc.infoText.setFillColor(sf::Color::Yellow);
                    
                    pc.acceptButton.setFont(ctx_.font);
                    pc.acceptButton.setText("Accept", 18);
                    pc.acceptButton.setSize(sf::Vector2f(140.f, 35.f));
                    pc.acceptButton.setColors(sf::Color(0, 120, 0), sf::Color::White, sf::Color::White);
                    // Callback removed - handled in handleEvent
                    
                    pc.rejectButton.setFont(ctx_.font);
                    pc.rejectButton.setText("Reject", 18);
                    pc.rejectButton.setSize(sf::Vector2f(140.f, 35.f));
                    pc.rejectButton.setColors(sf::Color(120, 0, 0), sf::Color::White, sf::Color::White);
                    // Callback removed - handled in handleEvent
                    
                    pendingChallenges_.push_back(std::move(pc));
                    rebuildChallengeList();
                    
                    pendingChallenges_.push_back(std::move(pc));
                    rebuildChallengeList();
                }
            } else if (type == MessageType::ACCEPT_CHALLENGE_RESPONSE ||
                       type == MessageType::REJECT_CHALLENGE_RESPONSE ||
                       type == MessageType::SEND_CHALLENGE_RESPONSE) {
                std::cout << "Challenge response received: " << msg.payload << "\n";
            }
        }
    }
}
