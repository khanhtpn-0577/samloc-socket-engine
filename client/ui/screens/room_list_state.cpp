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

RoomListState::RoomListState(StateContext& ctx) : ctx_(ctx), isPopupVisible_(false), isCreateRoomPopupVisible_(false), activeField_(CreateField::None), isDatCuoc_(true) {
    background_.setSize({1280, 720});
    background_.setFillColor(sf::Color(10, 45, 30)); 

    titleText_.setFont(ctx_.font);
    titleText_.setString("GAME LOBBY - SELECT ROOM");
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setStyle(sf::Text::Bold);
    
    sf::FloatRect tr = titleText_.getLocalBounds();
    titleText_.setOrigin(tr.width/2, tr.height/2);
    titleText_.setPosition(640.f, 40.f);

    btnBack_.setFont(ctx_.font);
    btnBack_.setText("BACK", 18);
    btnBack_.setSize({100, 40});
    btnBack_.setPosition({80, 40});
    btnBack_.setColors(sf::Color(60, 60, 60), sf::Color::White, sf::Color::White);
    btnBack_.setCallback([this]() { ctx_.requestTransition(GameStateType::Lobby); });

    btnLogout_.setFont(ctx_.font);
    btnLogout_.setText("LOGOUT", 18);
    btnLogout_.setSize({120, 40});
    btnLogout_.setPosition({1280.f - 120.f - 40.f, 40.f}); 
    btnLogout_.setColors(sf::Color(192, 57, 43), sf::Color::White, sf::Color::White);
    btnLogout_.setCallback([this]() {
        ctx_.network.authSender().sendLogout();
        ctx_.session.setLoggedIn(false);
        ctx_.requestTransition(GameStateType::Login);
    });

    btnCreateRoom_.setFont(ctx_.font);
    btnCreateRoom_.setText("CREATE PRIVATE ROOM", 18);
    btnCreateRoom_.setSize({260, 40});
    btnCreateRoom_.setPosition({980.f, 632.f});
    btnCreateRoom_.setColors(sf::Color(46, 204, 113), sf::Color::White, sf::Color::White);
    btnCreateRoom_.setCallback([this]() {
        openCreateRoomPopup();
    });

    createPopupBg_.setSize({620, 420});
    createPopupBg_.setOrigin(310, 210);
    createPopupBg_.setPosition(640, 360);
    createPopupBg_.setFillColor(sf::Color(32, 24, 18));
    createPopupBg_.setOutlineThickness(2.f);
    createPopupBg_.setOutlineColor(sf::Color(255, 215, 0, 200));

    createPopupHeader_.setSize({620, 70});
    createPopupHeader_.setOrigin(310, 210);
    createPopupHeader_.setPosition(640, 360);
    createPopupHeader_.setFillColor(sf::Color(255, 215, 0, 30));

    createPopupTitle_.setFont(ctx_.font);
    createPopupTitle_.setString("CREATE PRIVATE ROOM");
    createPopupTitle_.setCharacterSize(26);
    createPopupTitle_.setFillColor(sf::Color(255, 215, 0));
    createPopupTitle_.setStyle(sf::Text::Bold);
    {
        auto b = createPopupTitle_.getLocalBounds();
        createPopupTitle_.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        createPopupTitle_.setPosition(640.f, 245.f);
    }

    auto mkLabel = [&](sf::Text& t, const std::string& s, float x, float y) {
        t.setFont(ctx_.font);
        t.setString(s);
        t.setCharacterSize(18);
        t.setFillColor(sf::Color(220, 220, 220));
        t.setPosition(x, y);
    };

    mkLabel(lbRoomName_, "Room name", 420.f, 300.f);
    mkLabel(lbRoomType_, "Room type", 420.f, 365.f);
    mkLabel(lbBet_, "Bet", 420.f, 430.f);

    auto mkInputBox = [&](sf::RectangleShape& box, float x, float y) {
        box.setSize({360.f, 44.f});
        box.setPosition(x, y);
        box.setFillColor(sf::Color(20, 20, 20, 160));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color(255, 215, 0, 80));
    };

    mkInputBox(inputRoomNameBox_, 420.f, 325.f);
    mkInputBox(inputBetBox_, 420.f, 455.f);

    auto mkInputText = [&](sf::Text& t, float x, float y) {
        t.setFont(ctx_.font);
        t.setCharacterSize(18);
        t.setFillColor(sf::Color::White);
        t.setPosition(x + 12.f, y + 9.f);
    };
    mkInputText(inputRoomNameText_, 420.f, 325.f);
    mkInputText(inputBetText_, 420.f, 455.f);

    hintRoomName_.setFont(ctx_.font);
    hintRoomName_.setString("Enter room name...");
    hintRoomName_.setCharacterSize(16);
    hintRoomName_.setFillColor(sf::Color(180,180,180,160));
    hintRoomName_.setPosition(432.f, 336.f);

    hintBet_.setFont(ctx_.font);
    hintBet_.setString("Enter bet (digits only)...");
    hintBet_.setCharacterSize(16);
    hintBet_.setFillColor(sf::Color(180,180,180,160));
    hintBet_.setPosition(432.f, 466.f);

    btnTypeDatCuoc_.setFont(ctx_.font);
    btnTypeDatCuoc_.setText("DAT_CUOC", 16);
    btnTypeDatCuoc_.setSize({170.f, 40.f});
    btnTypeDatCuoc_.setPosition({420.f, 390.f});
    btnTypeDatCuoc_.setCallback([this]() { isDatCuoc_ = true; });

    btnTypeDemLa_.setFont(ctx_.font);
    btnTypeDemLa_.setText("DEM_LA", 16);
    btnTypeDemLa_.setSize({170.f, 40.f});
    btnTypeDemLa_.setPosition({610.f, 390.f});
    btnTypeDemLa_.setCallback([this]() { isDatCuoc_ = false; });

    btnCreateConfirm_.setFont(ctx_.font);
    btnCreateConfirm_.setText("CREATE", 18);
    btnCreateConfirm_.setSize({160.f, 48.f});
    btnCreateConfirm_.setPosition({420.f, 520.f});
    btnCreateConfirm_.setCallback([this]() { submitCreateRoom(); });

    btnCreateCancel_.setFont(ctx_.font);
    btnCreateCancel_.setText("CANCEL", 18);
    btnCreateCancel_.setSize({160.f, 48.f});
    btnCreateCancel_.setPosition({620.f, 520.f});
    btnCreateCancel_.setCallback([this]() { closeCreateRoomPopup(); });

    popupOverlay_.setSize({1280, 720});
    popupOverlay_.setFillColor(sf::Color(0, 0, 0, 200));

    popupBg_.setSize({600, 320});
    popupBg_.setOrigin(300, 160); 
    popupBg_.setPosition(640, 360);
    popupBg_.setFillColor(sf::Color(45, 30, 15));
    popupBg_.setOutlineThickness(3);
    popupBg_.setOutlineColor(sf::Color(255, 215, 0));

    popupText_.setFont(ctx_.font);
    popupText_.setCharacterSize(22); 
    popupText_.setFillColor(sf::Color::White);

    btnPopupOk_.setFont(ctx_.font);
    btnPopupOk_.setText("OK", 20);
    btnPopupOk_.setSize({120, 40});
    btnPopupOk_.setPosition({640, 430});
    btnPopupOk_.setColors(sf::Color(255, 215, 0), sf::Color::Black, sf::Color::Black);
    btnPopupOk_.setCallback([this]() { hidePopup(); });
}

void RoomListState::onEnter() {
    ctx_.roomHandler.setRoomListCallback([this](const std::vector<RoomInfo>& data) {
        this->onRoomsLoaded(data);
    });

    ctx_.roomHandler.setJoinRoomCallback([this](bool success, const std::string& msg, int id, const RoomInfo& info) {
        if (success) {
            this->ctx_.currentRoomInfo = info;
            this->ctx_.requestTransition(GameStateType::WaitingRoom);
        } else {
            this->showPopup(msg); 
        }
    });

    ctx_.roomHandler.setCreateRoomCallback(
        [this](bool success, const std::string& msg, int roomId, int roomCode) {
            if (success) {
                ctx_.network.roomSender().sendJoinRoom(roomId);
            } else {
                showPopup(msg.empty() ? "Create room failed" : msg);
            }
        }
    );

    ctx_.network.roomSender().sendGetRoomList();
}

void RoomListState::onExit() {
    ctx_.roomHandler.setRoomListCallback(nullptr);
    ctx_.roomHandler.setJoinRoomCallback(nullptr);
}

void RoomListState::onRoomsLoaded(const std::vector<RoomInfo>& rooms) {
    roomData_ = rooms;
    uiRooms_.clear();

    float startX = 80; float startY = 120;
    float cardW = 260; float cardH = 140;
    float gapX = 40; float gapY = 40;
    int cols = 4;

    for (size_t i = 0; i < rooms.size(); ++i) {
        RoomUIItem item;
        item.roomId = rooms[i].id;
        int row = i / cols;
        int col = i % cols;
        float x = startX + col * (cardW + gapX);
        float y = startY + row * (cardH + gapY);

        sf::Color headerColor;
        sf::Color bodyColor;

        if (rooms[i].type == "dat_cuoc") {
            bodyColor = sf::Color(80, 10, 10, 240); 
            headerColor = sf::Color(255, 215, 0); 
        } else {
            bodyColor = sf::Color(30, 60, 45, 240);
            headerColor = sf::Color(0, 255, 200); 
        }

        if (rooms[i].currentPlayers >= rooms[i].maxPlayers) {
            bodyColor = sf::Color(40, 40, 40, 240);
            headerColor = sf::Color(150, 150, 150);
        }
        
        item.baseColor = bodyColor;
        item.shape.setSize({cardW, cardH});
        item.shape.setPosition(x, y);
        item.shape.setFillColor(bodyColor);
        item.shape.setOutlineThickness(2);
        item.shape.setOutlineColor(headerColor);

        item.nameText.setFont(ctx_.font);
        item.nameText.setString(toSfString(rooms[i].name));
        item.nameText.setCharacterSize(20);
        item.nameText.setFillColor(headerColor);
        item.nameText.setStyle(sf::Text::Bold);
        item.nameText.setPosition(x + 15, y + 15);

        std::string modeStr = (rooms[i].type == "dat_cuoc") ? "[PRO]" : "[CASUAL]";
        std::string line1 = modeStr + " BET: " + formatMoney(rooms[i].bet);
        std::string line2 = "PLAYERS: " + std::to_string(rooms[i].currentPlayers) + "/" + std::to_string(rooms[i].maxPlayers);
        std::string line3 = (rooms[i].currentPlayers >= rooms[i].maxPlayers) ? "STATUS: FULL" : "STATUS: JOIN NOW";

        item.infoText.setFont(ctx_.font);
        item.infoText.setString(toSfString(line1 + "\n" + line2 + "\n" + line3));
        item.infoText.setCharacterSize(15);
        item.infoText.setFillColor(sf::Color(230, 230, 230));
        item.infoText.setPosition(x + 15, y + 50);

        uiRooms_.push_back(item);
    }
}

void RoomListState::onJoinResult(bool success, const std::string& message, int roomId, const RoomInfo& roomInfo) {
    if (success) {
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
    popupText_.setPosition(640, 340);
    isPopupVisible_ = true;
}

void RoomListState::hidePopup() {
    isPopupVisible_ = false;
}

void RoomListState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (isCreateRoomPopupVisible_) {
        handleCreatePopupEvent(event, mousePos);
        return;
    }

    if (isPopupVisible_) {
        btnPopupOk_.handleEvent(event, mousePos);
        return;
    }

    btnBack_.handleEvent(event, mousePos);
    btnLogout_.handleEvent(event, mousePos);
    btnCreateRoom_.handleEvent(event, mousePos);

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        for (const auto& item : uiRooms_) {
            if (item.shape.getGlobalBounds().contains(mousePos)) {
                lastClickedRoomId_ = item.roomId;
                ctx_.network.roomSender().sendJoinRoom(item.roomId);
                break;
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
    btnCreateRoom_.draw(window);

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    for (auto& item : uiRooms_) {
        if (!isPopupVisible_ && item.shape.getGlobalBounds().contains(mousePos)) {
            item.shape.setOutlineColor(sf::Color::Yellow);
            item.shape.setOutlineThickness(3);
        } else {
            sf::Color outline = (item.baseColor == sf::Color(80, 10, 10, 240)) ? sf::Color(255, 215, 0, 100) : sf::Color(0, 255, 200, 100);
            item.shape.setOutlineColor(outline);
            item.shape.setOutlineThickness(2);
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

    if (isCreateRoomPopupVisible_) {
        window.draw(popupOverlay_);
        window.draw(createPopupBg_);
        window.draw(createPopupHeader_);
        window.draw(createPopupTitle_);
        window.draw(lbRoomName_);
        window.draw(lbRoomType_);
        window.draw(lbBet_);
        window.draw(inputRoomNameBox_);
        window.draw(inputBetBox_);
        if (inputRoomName_.empty()) window.draw(hintRoomName_);
        if (inputBet_.empty()) window.draw(hintBet_);
        window.draw(inputRoomNameText_);
        window.draw(inputBetText_);
        btnTypeDatCuoc_.draw(window);
        btnTypeDemLa_.draw(window);
        btnCreateConfirm_.draw(window);
        btnCreateCancel_.draw(window);
    }
}

bool RoomListState::isDigitsOnly(const std::string& s) const {
    if (s.empty()) return false;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

void RoomListState::openCreateRoomPopup() {
    isCreateRoomPopupVisible_ = true;
    activeField_ = CreateField::RoomName;
    refreshCreatePopupTexts();
}

void RoomListState::closeCreateRoomPopup() {
    isCreateRoomPopupVisible_ = false;
    activeField_ = CreateField::None;
    refreshCreatePopupTexts();
}

void RoomListState::refreshCreatePopupTexts() {
    inputRoomNameText_.setString(toSfString(inputRoomName_));
    inputBetText_.setString(toSfString(inputBet_));
    auto focusColor = sf::Color(255, 215, 0, 220);
    auto normalColor = sf::Color(255, 215, 0, 80);
    inputRoomNameBox_.setOutlineColor(activeField_ == CreateField::RoomName ? focusColor : normalColor);
    inputBetBox_.setOutlineColor(activeField_ == CreateField::Bet ? focusColor : normalColor);
    if (isDatCuoc_) {
        btnTypeDatCuoc_.setColors(sf::Color(255, 215, 0), sf::Color::Black, sf::Color::Black);
        btnTypeDemLa_.setColors(sf::Color(60, 60, 60), sf::Color::White, sf::Color::White);
    } else {
        btnTypeDatCuoc_.setColors(sf::Color(60, 60, 60), sf::Color::White, sf::Color::White);
        btnTypeDemLa_.setColors(sf::Color(255, 215, 0), sf::Color::Black, sf::Color::Black);
    }
}

void RoomListState::handleCreatePopupEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (inputRoomNameBox_.getGlobalBounds().contains(mousePos)) {
            activeField_ = CreateField::RoomName;
            refreshCreatePopupTexts();
            return;
        }
        if (inputBetBox_.getGlobalBounds().contains(mousePos)) {
            activeField_ = CreateField::Bet;
            refreshCreatePopupTexts();
            return;
        }
    }

    btnTypeDatCuoc_.handleEvent(event, mousePos);
    btnTypeDemLa_.handleEvent(event, mousePos);
    btnCreateConfirm_.handleEvent(event, mousePos);
    btnCreateCancel_.handleEvent(event, mousePos);

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            closeCreateRoomPopup();
            return;
        }
        if (event.key.code == sf::Keyboard::Tab) {
            activeField_ = (activeField_ == CreateField::RoomName) ? CreateField::Bet : CreateField::RoomName;
            refreshCreatePopupTexts();
            return;
        }
        if (event.key.code == sf::Keyboard::Enter) {
            submitCreateRoom();
            return;
        }
        if (event.key.code == sf::Keyboard::BackSpace) {
            if (activeField_ == CreateField::RoomName && !inputRoomName_.empty()) {
                inputRoomName_.pop_back();
                refreshCreatePopupTexts();
                return;
            }
            if (activeField_ == CreateField::Bet && !inputBet_.empty()) {
                inputBet_.pop_back();
                refreshCreatePopupTexts();
                return;
            }
        }
    }

    if (event.type == sf::Event::TextEntered) {
        uint32_t u = event.text.unicode;
        if (u < 32 || u == 127) return; 
        char c = static_cast<char>(u);
        if (activeField_ == CreateField::RoomName) {
            if (inputRoomName_.size() < 20) {
                inputRoomName_.push_back(c);
                refreshCreatePopupTexts();
            }
            return;
        }
        if (activeField_ == CreateField::Bet) {
            if (c >= '0' && c <= '9' && inputBet_.size() < 12) {
                inputBet_.push_back(c);
                refreshCreatePopupTexts();
            }
            return;
        }
    }
}

void RoomListState::submitCreateRoom() {
    if (inputRoomName_.empty()) {
        showPopup("Room name is required");
        return;
    }
    if (inputBet_.empty() || !isDigitsOnly(inputBet_)) {
        showPopup("Bet must be digits only");
        return;
    }
    ctx_.network.roomSender().sendCreateRoomInfo(
        inputRoomName_,
        isDatCuoc_ ? "dat_cuoc" : "dem_la",
        std::stoll(inputBet_)
    );
    closeCreateRoomPopup();
}