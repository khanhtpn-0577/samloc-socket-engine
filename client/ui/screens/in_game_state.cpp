#include "in_game_state.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Helper chuyển đổi string UTF-8 sang SFML String
static sf::String toSf(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

// Format tiền tệ 1000 -> 1.000 $
static std::string formatMoney(long long value) {
    std::string s = std::to_string(value);
    int n = s.length() - 3;
    while (n > 0) { s.insert(n, "."); n -= 3; }
    return (s.empty() ? "0" : s) + " $";
}

// --- OPPONENT UI (Giao diện đối thủ) ---

void InGameState::OpponentUI::setup(sf::Font& font) {
    panelBg.setSize({170.f, 80.f});
    panelBg.setOrigin(85.f, 40.f);
    panelBg.setFillColor(sf::Color(15, 15, 15, 200)); 
    panelBg.setOutlineThickness(1.5f);
    panelBg.setOutlineColor(sf::Color(255, 255, 255, 20));

    avatarBg.setRadius(24.f);
    avatarBg.setOrigin(24.f, 24.f);
    avatarBg.setOutlineThickness(2.f);

    avatarLetter.setFont(font);
    avatarLetter.setCharacterSize(18);
    avatarLetter.setStyle(sf::Text::Bold);

    nameText.setFont(font);
    nameText.setCharacterSize(13);
    nameText.setFillColor(sf::Color(200, 200, 200));

    cardsLeftText.setFont(font);
    cardsLeftText.setCharacterSize(13);
    cardsLeftText.setFillColor(sf::Color(255, 215, 0));
}

void InGameState::OpponentUI::setContent(const RoomMember& member, int cardsCount, bool isTurn) {
    sf::Color avColors[] = { 
        sf::Color(180, 50, 50), sf::Color(50, 100, 180), 
        sf::Color(120, 50, 150), sf::Color(40, 150, 80) 
    };
    avatarBg.setFillColor(avColors[std::abs(member.id) % 4]);
    avatarLetter.setString(toSf(member.name.empty() ? "?" : std::string(1, std::toupper(member.name[0]))));
    nameText.setString(toSf(member.name));
    cardsLeftText.setString(toSf(std::to_string(cardsCount) + " CARDS"));

    if (isTurn) {
        panelBg.setOutlineColor(sf::Color(255, 215, 0)); 
        panelBg.setOutlineThickness(2.5f);
        panelBg.setFillColor(sf::Color(45, 40, 15, 255));
    } else {
        panelBg.setOutlineColor(sf::Color(255, 255, 255, 20));
        panelBg.setOutlineThickness(1.5f);
        panelBg.setFillColor(sf::Color(15, 15, 15, 200));
    }

    panelBg.setPosition(position);
    avatarBg.setPosition(position.x - 55.f, position.y);
    sf::FloatRect lRect = avatarLetter.getLocalBounds();
    avatarLetter.setOrigin(lRect.left + lRect.width/2.f, lRect.top + lRect.height/2.f);
    avatarLetter.setPosition(avatarBg.getPosition());
    nameText.setPosition(position.x - 10.f, position.y - 15.f);
    cardsLeftText.setPosition(position.x - 10.f, position.y + 10.f);
}

void InGameState::OpponentUI::setEmpty() { nameText.setString(toSf("EMPTY")); }

void InGameState::OpponentUI::draw(sf::RenderWindow& window) {
    if (nameText.getString() == "EMPTY") return;
    window.draw(panelBg); window.draw(avatarBg); window.draw(avatarLetter);
    window.draw(nameText); window.draw(cardsLeftText);
}

// --- IN GAME STATE ---

InGameState::InGameState(StateContext& ctx) : ctx_(ctx), visualTimeout_(0.f) {
    background_.setSize({1280.f, 720.f});
    background_.setFillColor(sf::Color(10, 30, 20)); 

    tableCenterBg_.setSize({600.f, 220.f});
    tableCenterBg_.setOrigin(300.f, 110.f);
    tableCenterBg_.setPosition(640.f, 310.f);
    tableCenterBg_.setFillColor(sf::Color(0, 0, 0, 100));
    tableCenterBg_.setOutlineThickness(1.f);
    tableCenterBg_.setOutlineColor(sf::Color(255, 255, 255, 10));

    roomInfoText_.setFont(ctx_.font);
    roomInfoText_.setCharacterSize(16);
    roomInfoText_.setFillColor(sf::Color(120, 140, 130));
    roomInfoText_.setPosition(30, 20);

    turnInfoText_.setFont(ctx_.font);
    turnInfoText_.setCharacterSize(26);
    turnInfoText_.setFillColor(sf::Color::White);
    turnInfoText_.setPosition(540.f, 40.f);
    
    // Nút PLAY: Quan trọng là phải gán setCallback
    btnPlay_.setFont(ctx_.font); 
    btnPlay_.setText("PLAY", 20); 
    btnPlay_.setSize({160.f, 55.f}); 
    btnPlay_.setPosition({1080.f, 580.f});
    btnPlay_.setCallback([this]() { this->onPlayClicked(); }); // ĐÃ GÁN CALLBACK

    // Nút PASS: Quan trọng là phải gán setCallback
    btnPass_.setFont(ctx_.font); 
    btnPass_.setText("PASS", 20); 
    btnPass_.setSize({160.f, 55.f}); 
    btnPass_.setPosition({1080.f, 645.f});
    btnPass_.setCallback([this]() { this->onPassClicked(); }); // ĐÃ GÁN CALLBACK

    setupOpponentLayout();
}

void InGameState::onEnter() {
    roomInfoText_.setString(toSf("BET: " + formatMoney(ctx_.currentRoomInfo.bet)));
    
    // Đăng ký nhận dữ liệu từ server
    ctx_.roomHandler.setRoomUpdateCallback([this](const std::vector<RoomMember>& m) { this->updateOpponentPlayers(m); });
    ctx_.roomHandler.setTurnInfoCallback([this](int id, int t) { this->updateTurnInfo(id, t); });
    ctx_.roomHandler.setMoveResultCallback([this](int u, const std::vector<int>& c, int l, const std::string& a) { this->updatePlayedCards(u, c, l, a); });
    
    syncHandFromServer();
}

void InGameState::update(float dt) {
    if (visualTimeout_ > 0) {
        visualTimeout_ -= dt;
        if (visualTimeout_ < 0) visualTimeout_ = 0;
        
        std::string currentName = (ctx_.currentPlayerTurnId == (int)ctx_.session.userId()) ? "YOUR TURN" : "OPPONENT";
        for(const auto& m : ctx_.currentRoomMembers) {
            if(m.id == ctx_.currentPlayerTurnId && m.id != (int)ctx_.session.userId()) {
                currentName = m.name; break;
            }
        }
        turnInfoText_.setString(toSf(currentName + " (" + std::to_string((int)std::ceil(visualTimeout_)) + "s)"));
    }
}

void InGameState::layoutHand() {
    if (myHand_.empty()) return;
    
    // KÍCH THƯỚC BÀI NHỎ HƠN
    float cardWidth = 80.f; 
    float gap = 45.f;      
    float totalWidth = cardWidth + (myHand_.size() - 1) * gap;
    float startX = (1280.f - totalWidth) / 2.0f;

    for (size_t i = 0; i < myHand_.size(); ++i) {
        float targetY = 610.f; 
        if (myHand_[i].isSelected()) targetY -= 35.f; 
        myHand_[i].setPosition(startX + i * gap, targetY);
    }
}

void InGameState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(tableCenterBg_);
    window.draw(roomInfoText_);
    window.draw(turnInfoText_);

    // Xử lý hover quân bài
    sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    int hoverIdx = -1;
    for (int i = (int)myHand_.size() - 1; i >= 0; --i) {
        if (myHand_[i].contains(mPos.x, mPos.y)) { hoverIdx = i; break; }
    }

    for (size_t i = 0; i < myHand_.size(); ++i) {
        myHand_[i].setHover((int)i == hoverIdx);
        window.draw(myHand_[i]);
    }

    // Bài đã đánh trên bàn (nhỏ và khít)
    float pX = 640.f - (playedCardsOnTable_.size() * 30.f / 2.f);
    for(size_t i=0; i<playedCardsOnTable_.size(); ++i) {
        playedCardsOnTable_[i].setPosition(pX + i * 30.f, 310.f);
        window.draw(playedCardsOnTable_[i]);
    }

    // Vẽ đối thủ
    for(int i=0; i<3; ++i) opponentSlots_[i].draw(window);

    // CẬP NHẬT TRẠNG THÁI NÚT TRƯỚC KHI VẼ
    bool isMyTurn = (ctx_.currentPlayerTurnId == (int)ctx_.session.userId());
    btnPlay_.setEnabled(isMyTurn);
    btnPass_.setEnabled(isMyTurn);

    btnPlay_.draw(window); 
    btnPass_.draw(window); 
}

void InGameState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    btnPlay_.handleEvent(event, mousePos);
    btnPass_.handleEvent(event, mousePos);

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        // Click chọn bài
        for (int i = (int)myHand_.size() - 1; i >= 0; --i) {
            if (myHand_[i].contains(mousePos.x, mousePos.y)) { 
                myHand_[i].toggleSelect(); 
                layoutHand();
                break; 
            }
        }
    }
}

void InGameState::updateOpponentPlayers(const std::vector<RoomMember>& members) {
    ctx_.currentRoomMembers = members; 
    int myId = (int)ctx_.session.userId();
    for(int i=0; i<3; ++i) opponentSlots_[i].setEmpty();
    int slotIdx = 0;
    for (const auto& m : members) {
        if (m.id == myId || m.id <= 0) continue;
        if (slotIdx < 3) {
            opponentSlots_[slotIdx].setContent(m, m.handSize, (m.id == ctx_.currentPlayerTurnId));
            slotIdx++;
        }
    }
}

void InGameState::updateTurnInfo(int currentPlayerId, int timeout) {
    ctx_.currentPlayerTurnId = currentPlayerId;
    visualTimeout_ = (float)timeout;
    updateOpponentPlayers(ctx_.currentRoomMembers);
}

void InGameState::updatePlayedCards(int userId, const std::vector<int>& cardsPlayed, int cardsLeft, const std::string& action) {
    if (action == "play" || action == "start") {
        playedCardsOnTable_.clear();
        for (int id : cardsPlayed) playedCardsOnTable_.emplace_back(id, ctx_.font);
    }
    syncHandFromServer();
}

void InGameState::syncHandFromServer() {
    int myId = (int)ctx_.session.userId();
    for (const auto& m : ctx_.currentRoomMembers) {
        if (m.id == myId) {
            myHand_.clear();
            for (int id : m.hand) myHand_.emplace_back(id, ctx_.font);
            break;
        }
    }
    // TỰ ĐỘNG SẮP XẾP BÀI
    std::sort(myHand_.begin(), myHand_.end(), [](const CardUI& a, const CardUI& b) { 
        return a.getID() < b.getID(); 
    });
    layoutHand();
}

void InGameState::onPlayClicked() {
    std::vector<int> selected;
    for (const auto& c : myHand_) {
        if (c.isSelected()) selected.push_back(c.getID());
    }
    
    if (selected.empty()) {
        std::cout << "[UI] No cards selected to play!" << std::endl;
        return;
    }
    
    // Gửi lệnh đánh bài lên Server
    ctx_.network.roomSender().sendPlayCards(selected);
}

void InGameState::onPassClicked() { 
    // Gửi lệnh bỏ lượt lên Server
    ctx_.network.roomSender().sendPassTurn(); 
}

void InGameState::onExit() {
    ctx_.roomHandler.setRoomUpdateCallback(nullptr);
    ctx_.roomHandler.setTurnInfoCallback(nullptr);
    ctx_.roomHandler.setMoveResultCallback(nullptr);
}

void InGameState::setupOpponentLayout() {
    for(int i=0; i<3; ++i) opponentSlots_[i].setup(ctx_.font);
    opponentSlots_[0].position = {1120.f, 320.f}; // Phải
    opponentSlots_[1].position = {640.f, 160.f};  // Trên
    opponentSlots_[2].position = {160.f, 320.f};  // Trái
}