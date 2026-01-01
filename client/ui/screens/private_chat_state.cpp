#include "private_chat_state.h"
#include <iostream>

PrivateChatState::PrivateChatState(StateContext& ctx)
    : ctx_(ctx),
      selectedFriendId_(0) {

    // ===== Panels =====
    leftPanel_.setSize({300.f, 720.f});
    leftPanel_.setFillColor(sf::Color(30, 30, 30));
    leftPanel_.setPosition(0.f, 0.f);

    rightPanel_.setSize({980.f, 720.f});
    rightPanel_.setFillColor(sf::Color(50, 50, 50));
    rightPanel_.setPosition(300.f, 0.f);

    // ===== Title =====
    titleText_.setFont(ctx_.font);
    titleText_.setString("Private Chat");
    titleText_.setCharacterSize(28);
    titleText_.setFillColor(sf::Color::White);
    titleText_.setPosition(20.f, 20.f);

    // ===== Chat title =====
    chatTitle_.setFont(ctx_.font);
    chatTitle_.setCharacterSize(22);
    chatTitle_.setFillColor(sf::Color::White);
    chatTitle_.setPosition(330.f, 20.f);

    // ===== Input =====
    messageInput_.setFont(ctx_.font);
    messageInput_.setPlaceholder("Type a message...");
    messageInput_.setPosition({330.f, 650.f});
    messageInput_.setSize({720.f, 40.f});
    messageInput_.setColors(
        sf::Color::White,
        sf::Color::Black,
        sf::Color::Black
    );

    // ===== Send button =====
    sendButton_.setFont(ctx_.font);
    sendButton_.setText("Send", 18);
    sendButton_.setSize({100.f, 40.f});
    sendButton_.setPosition({1060.f, 650.f});
    sendButton_.setColors(
        sf::Color(0, 120, 0),
        sf::Color::White,
        sf::Color::White
    );

    sendButton_.setCallback([this]() {
        if (selectedFriendId_ == 0) {
            std::cout << "[UI] No friend selected\n";
            return;
        }
        std::cout << "[UI] Send message to user "
                  << selectedFriendId_ << "\n";
        messageInput_.clear();
    });
}

void PrivateChatState::onEnter() {
    std::cout << "[PrivateChatState] Entered\n";
    buildDummyFriendList();
    rebuildFriendListLayout();
}

void PrivateChatState::onExit(){
    std::cout << "[PrivateChatState] Exited\n";
}

void PrivateChatState::handleEvent(const sf::Event& event, const sf::Vector2f &mousePos){

    messageInput_.handleEvent(event, mousePos);
    sendButton_.handleEvent(event, mousePos);

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {

        for (auto& f : friends_) {
            if (f.selectButton.isClicked(mousePos)) {
                selectedFriendId_ = f.userId;
                chatTitle_.setString("Chat with " + f.username);
                std::cout << "[UI] Selected friend: "
                          << f.username << "\n";
            }
        }
    }
}

void PrivateChatState::update(float dt){
    (void)dt;
}


void PrivateChatState::draw(sf::RenderWindow& window) {
    window.draw(leftPanel_);
    window.draw(rightPanel_);
    window.draw(titleText_);
    window.draw(chatTitle_);

    for (const auto& f : friends_) {
        window.draw(f.nameText);
        f.selectButton.draw(window);
    }

    messageInput_.draw(window);
    sendButton_.draw(window);
}


void PrivateChatState::buildDummyFriendList(){
    friends_.clear();

    for (uint32_t i = 1; i<=5; ++i){
        FriendItem f;
        f.userId = i;
        f.username = "Friend" + std::to_string(i);
        f.nameText.setFont(ctx_.font);  
        f.nameText.setString(f.username);
        f.nameText.setCharacterSize(18);
        f.nameText.setFillColor(sf::Color::White);

        f.selectButton.setFont(ctx_.font);
        f.selectButton.setText("Open", 14);
        f.selectButton.setSize({70.f, 28.f});
        f.selectButton.setColors(
            sf::Color(80, 80, 80),
            sf::Color::White,
            sf::Color::White
        );

        friends_.push_back(std::move(f));//chuyen f vao vector
    }
}

void PrivateChatState::rebuildFriendListLayout(){
    float y = 80.f;
    for (auto& f : friends_){
        f.nameText.setPosition(20.f, y);
        f.selectButton.setPosition({200.f, y - 5.f});
        y += 50.f;
    }
}
