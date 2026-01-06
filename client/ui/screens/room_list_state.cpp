#include "room_list_state.h"
#include <iostream>
#include <iomanip>

static sf::String toSfString(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

static std::string formatMoney(long long value) {
    std::string s = std::to_string(value);
    int n = s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + " $";
}

RoomListState::RoomListState(StateContext& ctx) : ctx_(ctx), isPopupVisible_(false) {
    background_.setSize({1280, 720});
    background_.setFillColor(sf::Color(26, 26, 46));

    titleText_.setFont(ctx_.font);
    titleText_.setString(toSfString("GAME LOBBY - SELECT ROOM")); // Tiếng Anh
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setStyle(sf::Text::Bold);
    
    sf::FloatRect tr = titleText_.getLocalBounds();
    titleText_.setOrigin(tr.width/2, tr.height/2);
    titleText_.setPosition(1280/2.0f, 30.f);

    btnBack_.setFont(ctx_.font);
    btnBack_.setText(toSfString("BACK"), 18); // Tiếng Anh
    btnBack_.setSize({100, 40});
    btnBack_.setPosition({30, 10});
    btnBack_.setColors(sf::Color(100, 100, 100), sf::Color::White, sf::Color::White);
    btnBack_.setCallback([this]() { ctx_.requestTransition(GameStateType::Lobby); });

    btnLogout_.setFont(ctx_.font);
    btnLogout_.setText(toSfString("LOGOUT"), 18); // Tiếng Anh
    btnLogout_.setSize({120, 40});
    btnLogout_.setPosition({1280.f - 120.f - 30.f, 10.f});
    btnLogout_.setColors(sf::Color(192, 57, 43), sf::Color::White, sf::Color::White);
    btnLogout_.setCallback([this]() {
        ctx_.network.authSender().sendLogout();
        ctx_.session.setLoggedIn(false);
        ctx_.requestTransition(GameStateType::Login);
    });

    popupOverlay_.setSize({1280, 720});
    popupOverlay_.setFillColor(sf::Color(0, 0, 0, 200));

    popupBg_.setSize({500, 250});
    popupBg_.setOrigin(250, 125);
    popupBg_.setPosition(640, 360);
    popupBg_.setFillColor(sf::Color(20, 20, 20));
    popupBg_.setOutlineThickness(2);
    popupBg_.setOutlineColor(sf::Color(255, 0, 0));

    popupText_.setFont(ctx_.font);
    popupText_.setCharacterSize(20);
    popupText_.setFillColor(sf::Color::White);

    btnPopupOk_.setFont(ctx_.font);
    btnPopupOk_.setText("OK", 20);
    btnPopupOk_.setSize({120, 40});
    btnPopupOk_.setPosition({640 - 60, 430});
    btnPopupOk_.setColors(sf::Color(200, 50, 50), sf::Color::White, sf::Color::White);
    btnPopupOk_.setCallback([this]() { hidePopup(); });
}

void RoomListState::onEnter() {
    ctx_.roomHandler.setRoomListCallback([this](const std::vector<RoomInfo>& data) {
        this->onRoomsLoaded(data);
    });

    ctx_.roomHandler.setJoinRoomCallback([this](bool success, const std::string& msg, int roomId, const RoomInfo& roomInfo) {
        this->onJoinResult(success, msg, roomId, roomInfo);
    });

    ctx_.network.roomSender().sendGetRoomList();
}

void RoomListState::onExit() {
    ctx_.roomHandler.setRoomListCallback(nullptr);
    ctx_.roomHandler.setJoinRoomCallback(nullptr);
}

void RoomListState::onRoomsLoaded(const std::vector<RoomInfo>& rooms) {
    roomData_ = rooms;
    uiRooms_.clear();

    float startX = 80;
    float startY = 120;
    float cardW = 260;
    float cardH = 140;
    float gapX = 40;
    float gapY = 40;
    int cols = 4;

    for (size_t i = 0; i < rooms.size(); ++i) {
        RoomUIItem item;
        item.roomId = rooms[i].id;

        int row = i / cols;
        int col = i % cols;
        float x = startX + col * (cardW + gapX);
        float y = startY + row * (cardH + gapY);

        sf::Color bgCol = (rooms[i].type == "dat_cuoc") ? sf::Color(22, 160, 133) : sf::Color(142, 68, 173);
        if (rooms[i].currentPlayers >= rooms[i].maxPlayers) bgCol = sf::Color(100, 30, 30);
        
        item.baseColor = bgCol;
        item.shape.setSize({cardW, cardH});
        item.shape.setPosition(x, y);
        item.shape.setFillColor(bgCol);
        item.shape.setOutlineThickness(2);
        item.shape.setOutlineColor(sf::Color::White);

        item.nameText.setFont(ctx_.font);
        item.nameText.setString(toSfString(rooms[i].name));
        item.nameText.setCharacterSize(20);
        item.nameText.setStyle(sf::Text::Bold);
        item.nameText.setPosition(x + 15, y + 15);

        std::string line1 = "Bet: " + formatMoney(rooms[i].bet); // Tiếng Anh
        std::string line2 = "Players: " + std::to_string(rooms[i].currentPlayers) + "/" + std::to_string(rooms[i].maxPlayers);
        std::string line3 = (rooms[i].currentPlayers >= rooms[i].maxPlayers) ? "FULL" : "WAITING"; // Tiếng Anh

        item.infoText.setFont(ctx_.font);
        item.infoText.setString(toSfString(line1 + "\n" + line2 + "\n" + line3));
        item.infoText.setCharacterSize(16);
        item.infoText.setFillColor(sf::Color(220, 220, 220));
        item.infoText.setPosition(x + 15, y + 55);

        uiRooms_.push_back(item);
    }
}

void RoomListState::onJoinResult(bool success, const std::string& message, int roomId, const RoomInfo& roomInfo) {
    if (success) {
        RoomInfo actualRoomInfo;
        for(const auto& r : roomData_) {
            if (r.id == roomId) {
                actualRoomInfo = r;
                break;
            }
        }
        ctx_.currentRoomInfo = actualRoomInfo;
        ctx_.currentRoomId = roomId; 
        
        ctx_.requestTransition(GameStateType::WaitingRoom);
    } else {
        showPopup(message);
    }
}

void RoomListState::showPopup(const std::string& msg) {
    popupText_.setString(toSfString(msg));
    
    sf::FloatRect tr = popupText_.getLocalBounds();
    popupText_.setOrigin(tr.left + tr.width/2, tr.top + tr.height/2);
    popupText_.setPosition(640, 360 - 20);
    
    isPopupVisible_ = true;
}

void RoomListState::hidePopup() {
    isPopupVisible_ = false;
}

void RoomListState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (isPopupVisible_) {
        btnPopupOk_.handleEvent(event, mousePos);
        return;
    }

    btnBack_.handleEvent(event, mousePos);
    btnLogout_.handleEvent(event, mousePos);

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        for (const auto& item : uiRooms_) {
            if (item.shape.getGlobalBounds().contains(mousePos)) {
                lastClickedRoomId_ = item.roomId;
                ctx_.network.roomSender().sendJoinRoom(item.roomId);
            }
        }
    }
}

void RoomListState::update(float dt) { (void)dt; }

void RoomListState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(titleText_);
    btnBack_.draw(window);
    btnLogout_.draw(window);

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    for (auto& item : uiRooms_) {
        if (!isPopupVisible_ && item.shape.getGlobalBounds().contains(mousePos)) {
            item.shape.setFillColor(item.baseColor + sf::Color(30, 30, 30));
            item.shape.setOutlineColor(sf::Color::Yellow);
            item.shape.setScale(1.02f, 1.02f);
        } else {
            item.shape.setFillColor(item.baseColor);
            item.shape.setOutlineColor(sf::Color(100, 100, 100));
            item.shape.setScale(1.0f, 1.0f);
        }
        window.draw(item.shape);
        window.draw(item.nameText);
        window.draw(item.infoText);
    }

    if (isPopupVisible_) {
        window.draw(popupOverlay_);
        window.draw(popupBg_);
        window.draw(popupText_);
        btnPopupOk_.draw(window);
    }
}