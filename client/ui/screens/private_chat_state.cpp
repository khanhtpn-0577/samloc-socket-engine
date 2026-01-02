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

        std::string message = messageInput_.value();
        if (message.empty()) {
            std::cout << "[UI] Empty message, ignore\n";
            return;
        }

        std::cout << "[UI] Send message to user "
                << selectedFriendId_
                << ": " << message << "\n";

        ctx_.chatHandler.onSendPrivateChat(
            selectedFriendId_,
            message
        );

        ChatBubble bubble;
        bubble.senderId = ctx_.session.userId();

        bubble.text.setFont(ctx_.font);
        bubble.text.setCharacterSize(16);
        bubble.text.setFillColor(sf::Color::Green);
        bubble.text.setString(message);

        chatBubbles_.push_back(std::move(bubble));
        rebuildChatLayout();

        messageInput_.clear();
    });

}

void PrivateChatState::onEnter() {
    std::cout << "[PrivateChatState] Entered\n";

    //Register callback nhận friend list
    ctx_.chatHandler.setFriendListCallback(
        [this](const std::vector<FriendInfo>& friends) {
            std::cout << "[UI] Friend list received: "
                      << friends.size() << " friends\n";
            buildFriendListFromData(friends);
        }
    );

    // Chat history callback
    ctx_.chatHandler.setChatHistoryCallback(
        [this](const std::vector<ChatHistoryItem>& history) {
            buildChatHistory(history);
        }
    );

    ctx_.chatHandler.setIncomingMessageCallback(
        [this](uint32_t senderId, const std::string& content) {

            // Chỉ render nếu đang chat với người này
            if (senderId != selectedFriendId_) {
                std::cout << "[UI] Message from other user, ignore\n";
                return;
            }

            ChatBubble bubble;
            bubble.senderId = senderId;
            bubble.text.setFont(ctx_.font);
            bubble.text.setString(content);
            bubble.text.setCharacterSize(16);
            bubble.text.setFillColor(sf::Color::White);

            chatBubbles_.push_back(std::move(bubble));
            rebuildChatLayout();
        }
    );


    //Request friend list từ server
    requestFriendList();
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

                ctx_.chatHandler.requestPrivateChatHistory(selectedFriendId_);

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

    for (const auto& bubble : chatBubbles_) {
        window.draw(bubble.text);
    }

    messageInput_.draw(window);
    sendButton_.draw(window);
}

void PrivateChatState::rebuildFriendListLayout(){
    float y = 80.f;
    for (auto& f : friends_){
        f.nameText.setPosition(20.f, y);
        f.selectButton.setPosition({200.f, y - 5.f});
        y += 50.f;
    }
}

void PrivateChatState::requestFriendList() {
    std::cout << "[UI] Request friend list\n";
    ctx_.chatHandler.requestFriendList();
}

void PrivateChatState::buildFriendListFromData(
    const std::vector<FriendInfo>& friends
) {
    friends_.clear();

    for (const auto& info : friends) {
        FriendItem f;
        f.userId = info.userId;
        f.username = info.username;

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

        friends_.push_back(std::move(f));
    }

    rebuildFriendListLayout();
}

void PrivateChatState::buildChatHistory(
    const std::vector<ChatHistoryItem>& history
) {
    chatBubbles_.clear();

    float y = 80.f;

    for (const auto& item : history) {
        ChatBubble bubble;
        bubble.senderId = item.senderId;

        bubble.text.setFont(ctx_.font);
        bubble.text.setCharacterSize(16);
        bubble.text.setFillColor(
            item.senderId == ctx_.session.userId()
                ? sf::Color::Green
                : sf::Color::White
        );

        bubble.text.setString(item.content);

        float x =
            item.senderId == ctx_.session.userId()
                ? 720.f
                : 330.f;

        bubble.text.setPosition(x, y);
        y += 26.f;

        chatBubbles_.push_back(std::move(bubble));
    }
}

void PrivateChatState::rebuildChatLayout() {
    float y = 80.f;
    for (auto& bubble : chatBubbles_) {
        float x = (bubble.senderId == ctx_.session.userId())
                ? 950.f - bubble.text.getLocalBounds().width
                : 330.f;
        bubble.text.setPosition(x, y);
        y += 30.f;
    }
}


