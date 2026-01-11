#include "waiting_room_state.h"
#include <iostream>

namespace UITheme {
    static const sf::Color Background = sf::Color(10, 45, 30);      
    static const sf::Color Panel = sf::Color(45, 30, 15, 230);  
    static const sf::Color Gold = sf::Color(255, 215, 0);          
    static const sf::Color TextMain = sf::Color::White;
    static const sf::Color TextSub = sf::Color(180, 180, 180);
    static const sf::Color ReadyGreen = sf::Color(39, 174, 96);   
    static const sf::Color CancelOrange = sf::Color(211, 84, 0);   
    static const sf::Color LeaveRed = sf::Color(192, 57, 43);
    static const sf::Color SlotOutline = sf::Color(255, 215, 0, 60);
}

static sf::String toSf(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

static std::string formatMoney(long long value) {
    std::string s = std::to_string(value);
    int n = (int)s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + " $";
}

static std::string utf8_first_char(const std::string& s) {
    if (s.empty()) return "?";
    unsigned char c = static_cast<unsigned char>(s[0]);
    int len = 1;
    if ((c & 0x80) == 0x00) len = 1;
    else if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xF0) == 0xE0) len = 3;
    else len = 4;
    return s.size() >= (size_t)len ? s.substr(0, len) : "?";
}

static void centerText(sf::Text& t, float x, float y) {
    sf::FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width / 2.0f, r.top + r.height / 2.0f);
    t.setPosition(x, y);
}

void WaitingRoomState::PlayerSlot::setup(sf::Font& font) {
    panelBg.setSize({220.f, 140.f});
    panelBg.setOrigin(110.f, 70.f);
    panelBg.setFillColor(UITheme::Panel);
    panelBg.setOutlineThickness(2.0f);
    panelBg.setOutlineColor(UITheme::SlotOutline);

    avatarBg.setRadius(32.f);
    avatarBg.setOrigin(32.f, 32.f);
    avatarBg.setOutlineThickness(2.f);
    avatarBg.setOutlineColor(UITheme::Gold);

    avatarLetter.setFont(font);
    avatarLetter.setCharacterSize(28);
    avatarLetter.setFillColor(UITheme::TextMain);

    nameText.setFont(font);
    nameText.setCharacterSize(17);
    nameText.setFillColor(UITheme::TextMain);

    balanceText.setFont(font);
    balanceText.setCharacterSize(14);
    balanceText.setFillColor(UITheme::Gold);

    readyStatusText.setFont(font);
    readyStatusText.setCharacterSize(13);
}

void WaitingRoomState::PlayerSlot::setContent(const RoomMember& member) {
    static sf::Color avColors[] = {
        sf::Color(9, 132, 227), sf::Color(0, 184, 148),
        sf::Color(108, 92, 231), sf::Color(225, 112, 85)
    };
    avatarBg.setFillColor(avColors[std::abs(member.id) % 4]);
    std::string initial = member.name.empty() ? "?" : utf8_first_char(member.name);
    avatarLetter.setString(toSf(initial));
    nameText.setString(toSf(member.name));
    balanceText.setString(toSf(formatMoney(member.balance)));
    
    if (member.isReady) {
        readyStatusText.setString(toSf("READY"));
        readyStatusText.setFillColor(UITheme::ReadyGreen);
        panelBg.setOutlineColor(UITheme::ReadyGreen);
    } else {
        readyStatusText.setString(toSf("WAITING..."));
        readyStatusText.setFillColor(UITheme::TextSub);
        panelBg.setOutlineColor(UITheme::SlotOutline);
    }
    
    panelBg.setPosition(position);
    avatarBg.setPosition(position.x, position.y - 45.f);
    centerText(avatarLetter, avatarBg.getPosition().x, avatarBg.getPosition().y);
    centerText(nameText, position.x, position.y + 12.f);
    centerText(balanceText, position.x, position.y + 35.f);
    centerText(readyStatusText, position.x, position.y + 55.f);
}

void WaitingRoomState::PlayerSlot::setEmpty() {
    panelBg.setOutlineColor(sf::Color(255, 215, 0, 30));
    panelBg.setFillColor(sf::Color(0, 0, 0, 120));
    panelBg.setPosition(position);
    avatarBg.setFillColor(sf::Color(30, 30, 30, 150));
    avatarBg.setOutlineColor(sf::Color(255, 255, 255, 20));
    avatarBg.setPosition(position.x, position.y - 45.f);
    avatarLetter.setString(toSf("+"));
    centerText(avatarLetter, avatarBg.getPosition().x, avatarBg.getPosition().y);
    nameText.setString(toSf("EMPTY"));
    nameText.setFillColor(sf::Color(100, 100, 100));
    centerText(nameText, position.x, position.y + 12.f);
    balanceText.setString("");
    readyStatusText.setString("");
}

void WaitingRoomState::PlayerSlot::draw(sf::RenderWindow& w) {
    w.draw(panelBg);
    w.draw(avatarBg);
    w.draw(avatarLetter);
    w.draw(nameText);
    w.draw(balanceText);
    w.draw(readyStatusText);
}

WaitingRoomState::WaitingRoomState(StateContext& ctx)
    : ctx_(ctx), isMyReady_(false) {
    background_.setSize({1280, 720});
    background_.setFillColor(UITheme::Background);
    
    headerBar_.setSize({1280, 56});
    headerBar_.setFillColor(sf::Color(0, 0, 0, 200));
    
    roomNameText_.setFont(ctx_.font);
    roomNameText_.setCharacterSize(24);
    roomNameText_.setFillColor(UITheme::Gold);
    roomNameText_.setPosition(25, 12);
    
    subTitleText_.setFont(ctx_.font);
    subTitleText_.setCharacterSize(14);
    subTitleText_.setFillColor(UITheme::TextSub);
    subTitleText_.setPosition(400, 20);
    
    btnLeave_.setFont(ctx_.font);
    btnLeave_.setText("LEAVE ROOM", 16);
    btnLeave_.setSize({140, 36});
    btnLeave_.setPosition({1120, 10});
    btnLeave_.setColors(UITheme::LeaveRed, sf::Color::White, sf::Color::White);
    btnLeave_.setCallback([this]{ onLeaveClicked(); });
    
    btnReady_.setFont(ctx_.font);
    btnReady_.setText("READY", 22);
    btnReady_.setSize({220, 60});
    btnReady_.setPosition({640.f, 620.f}); 
    btnReady_.setColors(UITheme::ReadyGreen, sf::Color::White, sf::Color::White);
    btnReady_.setCallback([this]{ onReadyClicked(); });
    
    btnChat_.setFont(ctx_.font);
    btnChat_.setText("CHAT", 14);
    btnChat_.setSize({60, 60});
    btnChat_.setPosition({1190, 620});
    btnChat_.setColors(sf::Color(41, 128, 185), sf::Color::White, sf::Color::White);

    btnInviteFriend_.setFont(ctx_.font);
    btnInviteFriend_.setText("INVITE", 14);
    btnInviteFriend_.setSize({100, 36});
    btnInviteFriend_.setPosition({1000, 10}); // góc phải trên, cạnh LEAVE
    btnInviteFriend_.setColors(
        sf::Color(52, 152, 219),   // xanh invite
        sf::Color::White,
        sf::Color::White
    );
    btnInviteFriend_.setCallback([this] {
        std::cout << "[WaitingRoom] Invite Friend clicked\n";
        ctx_.friendHandler.onRequestFriendList(ctx_.session.userId());
    });


    // ===== Invite Friend Popup =====
    invitePopupBg_.setSize({600, 400});
    invitePopupBg_.setOrigin(300, 200);
    invitePopupBg_.setPosition(640, 360);
    invitePopupBg_.setFillColor(sf::Color(20, 20, 20, 240));
    invitePopupBg_.setOutlineThickness(2);
    invitePopupBg_.setOutlineColor(UITheme::Gold);

    invitePopupTitle_.setFont(ctx_.font);
    invitePopupTitle_.setCharacterSize(22);
    invitePopupTitle_.setFillColor(UITheme::Gold);
    invitePopupTitle_.setString(toSf("INVITE FRIEND"));
    centerText(invitePopupTitle_, 640, 220);

    invitePopupListText_.setFont(ctx_.font);
    invitePopupListText_.setCharacterSize(16);
    invitePopupListText_.setFillColor(UITheme::TextMain);
    invitePopupListText_.setPosition(380, 260);

    btnInvitePopupClose_.setFont(ctx_.font);
    btnInvitePopupClose_.setText("CLOSE", 16);
    btnInvitePopupClose_.setSize({120, 36});
    btnInvitePopupClose_.setPosition({640, 520});
    btnInvitePopupClose_.setColors(UITheme::LeaveRed, sf::Color::White, sf::Color::White);
    btnInvitePopupClose_.setCallback([this] {
        isInvitePopupVisible_ = false;
    });

    //popup invite result
    challengePopupBg_.setSize({420, 200});
    challengePopupBg_.setOrigin(210, 100);
    challengePopupBg_.setPosition(640, 360);
    challengePopupBg_.setFillColor(sf::Color(20, 20, 20, 240));
    challengePopupBg_.setOutlineThickness(2);
    challengePopupBg_.setOutlineColor(UITheme::Gold);

    challengePopupText_.setFont(ctx_.font);
    challengePopupText_.setCharacterSize(18);
    challengePopupText_.setFillColor(UITheme::TextMain);
    centerText(challengePopupText_, 640, 340);

    challengePopupOkBtn_.setFont(ctx_.font);
    challengePopupOkBtn_.setText("OK", 16);
    challengePopupOkBtn_.setSize({120, 36});
    challengePopupOkBtn_.setPosition({640, 420});
    challengePopupOkBtn_.setColors(
        sf::Color(52, 152, 219),
        sf::Color::White,
        sf::Color::White
    );
    challengePopupOkBtn_.setCallback([this] {
        isChallengePopupVisible_ = false;
    });


    
    setupSlotsLayout();
}

void WaitingRoomState::setupSlotsLayout() {
    for (int i = 0; i < 4; ++i) slots_[i].setup(ctx_.font);
    slots_[0].position = {640.f, 480.f}; 
    slots_[1].position = {1000.f, 320.f}; 
    slots_[2].position = {640.f, 160.f};  
    slots_[3].position = {280.f, 320.f}; 
}

void WaitingRoomState::onEnter() {
    isMyReady_ = false;
    members_.clear();
    
    currentRoomInfo_ = ctx_.currentRoomInfo; 
    
    roomNameText_.setString(toSf("ROOM: " + currentRoomInfo_.name));
    
    long long displayBet = currentRoomInfo_.bet;
    if (displayBet < 0 || displayBet > 1000000000000LL) displayBet = 0; 
    
    subTitleText_.setString(toSf("Min Bet: " + formatMoney(displayBet) + " | Mode: " + currentRoomInfo_.type));
    
    ctx_.roomHandler.setRoomUpdateCallback([this](const std::vector<RoomMember>& newMembers) {
        this->updateMembers(newMembers);
    });
    
    ctx_.roomHandler.setGameCountdownCallback([this](int seconds) {
        ctx_.requestTransition(GameStateType::GameStartingCountdown);
    });

    ctx_.roomHandler.setGameStartCallback([this](const std::vector<int>& hand){
        ctx_.myHand = hand;
        ctx_.requestTransition(GameStateType::InGame);
    });

    ctx_.friendHandler.setFriendListCallback(
        [this](const std::vector<
            std::pair<uint32_t,
            std::pair<std::string, std::pair<double, bool>>>>& friends) {

            inviteFriendRows_.clear();

            float startY = 260.f;
            float rowHeight = 48.f;

            for (size_t i = 0; i < friends.size(); ++i) {
                const auto& f = friends[i];

                InviteFriendRow row;
                row.userId = f.first;
                row.username = f.second.first;
                row.balance = (long long)f.second.second.first;
                row.online = f.second.second.second;

                // === Background ===
                row.bg.setSize({520.f, 42.f});
                row.bg.setPosition(380.f, startY + i * rowHeight);
                row.bg.setFillColor(sf::Color(30, 30, 30, 220));
                row.bg.setOutlineThickness(1.f);
                row.bg.setOutlineColor(
                    row.online ? UITheme::ReadyGreen : sf::Color(120,120,120)
                );

                // === Username ===
                row.nameText.setFont(ctx_.font);
                row.nameText.setCharacterSize(16);
                row.nameText.setFillColor(UITheme::TextMain);
                row.nameText.setString(toSf(row.username));
                row.nameText.setPosition(395.f, startY + i * rowHeight + 10.f);

                // === Balance ===
                row.balanceText.setFont(ctx_.font);
                row.balanceText.setCharacterSize(14);
                row.balanceText.setFillColor(UITheme::Gold);
                row.balanceText.setString(toSf(formatMoney(row.balance)));
                row.balanceText.setPosition(560.f, startY + i * rowHeight + 12.f);

                // === Status ===
                row.statusText.setFont(ctx_.font);
                row.statusText.setCharacterSize(14);
                row.statusText.setString(row.online ? "ONLINE" : "OFFLINE");
                row.statusText.setFillColor(
                    row.online ? UITheme::ReadyGreen : UITheme::TextSub
                );
                row.statusText.setPosition(720.f, startY + i * rowHeight + 12.f);

                // === Invite Button ===
                if (row.online) {
                    row.inviteBtn.setFont(ctx_.font);
                    row.inviteBtn.setText("INVITE", 14);
                    row.inviteBtn.setSize({80, 30});
                    row.inviteBtn.setPosition({820.f, startY + i * rowHeight + 6.f});
                    row.inviteBtn.setColors(
                        sf::Color(52, 152, 219),
                        sf::Color::White,
                        sf::Color::White
                    );
                    row.inviteBtn.setCallback(
                        [this, uid = row.userId] {
                            std::cout << "[Invite] Send challenge to userId=" << uid
                                    << " roomId=" << ctx_.currentRoomId << "\n";

                            ctx_.challengeHandler.onSendChallenge(
                                uid,
                                ctx_.currentRoomId
                            );
                        }
                    );

                }

                inviteFriendRows_.push_back(row);
            }

            isInvitePopupVisible_ = true;
        }
    );

    ctx_.challengeHandler.setChallengeResultCallback(
        [this](bool success, const std::string& msg) {
            challengeResultSuccess_ = success;
            challengePopupText_.setString(
                toSf(success ? "INVITE SENT!\n" + msg
                            : "INVITE FAILED!\n" + msg)
            );
            challengePopupText_.setFillColor(
                success ? UITheme::ReadyGreen : UITheme::LeaveRed
            );
            centerText(challengePopupText_, 640, 340);
            isChallengePopupVisible_ = true;
        }
    );



}

void WaitingRoomState::onExit() {
    ctx_.roomHandler.setRoomUpdateCallback(nullptr);
    ctx_.roomHandler.setGameCountdownCallback(nullptr);
    ctx_.roomHandler.setGameStartCallback(nullptr);
    ctx_.friendHandler.setFriendListCallback(nullptr);
}

void WaitingRoomState::updateMembers(const std::vector<RoomMember>& newMembers) {
    members_ = newMembers;
    refreshSlotDisplay();
}

void WaitingRoomState::refreshSlotDisplay() {
    int myId = (int)ctx_.session.userId();
    int myIndex = -1;
    for (size_t i = 0; i < members_.size(); ++i) {
        if (members_[i].id == myId) {
            myIndex = (int)i;
            break;
        }
    }
    if (myIndex == -1) {
        for (int i = 0; i < 4; ++i) slots_[i].setEmpty();
        return;
    }
    isMyReady_ = members_[myIndex].isReady;
    btnReady_.setText(toSf(isMyReady_ ? "UNREADY" : "READY"));
    btnReady_.setColors(isMyReady_ ? UITheme::CancelOrange : UITheme::ReadyGreen, sf::Color::White, sf::Color::White);
    
    slots_[0].setContent(members_[myIndex]);
    int uiSlot = 1;
    for (size_t i = 0; i < members_.size() && uiSlot < 4; ++i) {
        if ((int)i == myIndex) continue;
        slots_[uiSlot++].setContent(members_[i]);
    }
    for (; uiSlot < 4; ++uiSlot) slots_[uiSlot].setEmpty();
}

void WaitingRoomState::onReadyClicked() {
    isMyReady_ = !isMyReady_;
    ctx_.network.roomSender().sendReady(isMyReady_);
}

void WaitingRoomState::onLeaveClicked() {
    ctx_.network.roomSender().sendLeaveRoom();
    ctx_.requestTransition(GameStateType::Lobby);
}

void WaitingRoomState::handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) {
    if (isChallengePopupVisible_) {
        challengePopupOkBtn_.handleEvent(e, mousePos);
        return;
    }
    
    if (isInvitePopupVisible_) {
        btnInvitePopupClose_.handleEvent(e, mousePos);
        for (auto& row : inviteFriendRows_) {
            if (row.online)
                row.inviteBtn.handleEvent(e, mousePos);
        }
        return;
    }

    btnLeave_.handleEvent(e, mousePos);
    btnReady_.handleEvent(e, mousePos);
    btnChat_.handleEvent(e, mousePos);
    btnInviteFriend_.handleEvent(e, mousePos);
}

void WaitingRoomState::update(float dt) { (void)dt; }

void WaitingRoomState::draw(sf::RenderWindow& w) {
    w.draw(background_);
    w.draw(headerBar_);
    w.draw(roomNameText_);
    w.draw(subTitleText_);
    for (int i = 0; i < 4; ++i) slots_[i].draw(w);
    btnLeave_.draw(w);
    btnReady_.draw(w);
    btnChat_.draw(w);
    btnInviteFriend_.draw(w);

    if (isInvitePopupVisible_) {
        sf::RectangleShape overlay;
        overlay.setSize({1280, 720});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        w.draw(overlay);

        w.draw(invitePopupBg_);
        w.draw(invitePopupTitle_);

        // === DRAW FRIEND ROWS ===
        for (auto& row : inviteFriendRows_) {
            w.draw(row.bg);
            w.draw(row.nameText);
            w.draw(row.balanceText);
            w.draw(row.statusText);

            if (row.online) {
                row.inviteBtn.draw(w);
            }
        }

        btnInvitePopupClose_.draw(w);
    }

    if (isChallengePopupVisible_) {
        sf::RectangleShape overlay;
        overlay.setSize({1280, 720});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        w.draw(overlay);

        w.draw(challengePopupBg_);
        w.draw(challengePopupText_);
        challengePopupOkBtn_.draw(w);
    }



}