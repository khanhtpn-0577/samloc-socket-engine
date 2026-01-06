#include "waiting_room_state.h"
#include <iostream>

/* ================= HELPER FUNCTIONS ================= */

// Chuyển đổi an toàn từ std::string (UTF-8) sang sf::String để hiển thị tiếng Việt
static sf::String toSf(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

// Định dạng tiền tệ: 1000000 -> 1.000.000 $
static std::string formatMoney(long long value) {
    std::string s = std::to_string(value);
    int n = (int)s.length() - 3;
    while (n > 0) {
        s.insert(n, ".");
        n -= 3;
    }
    return s + " $";
}

// Lấy ký tự đầu tiên chuẩn UTF-8 (hỗ trợ các chữ có dấu như Ă, Â, Đ...)
static std::string utf8_first_char(const std::string& s) {
    if (s.empty()) return "?";
    unsigned char c = static_cast<unsigned char>(s[0]);
    int len = 1;
    if ((c & 0x80) == 0x00) len = 1;
    else if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xE0) == 0xE0) len = 3;
    else if ((c & 0xF0) == 0xF0) len = 4;
    if ((int)s.size() < len) return "?";
    return s.substr(0, len);
}

// Hàm bổ trợ căn giữa Text chuyên nghiệp
static void centerText(sf::Text& t, float x, float y) {
    sf::FloatRect r = t.getLocalBounds();
    t.setOrigin(r.left + r.width / 2.0f, r.top + r.height / 2.0f);
    t.setPosition(x, y);
}

/* ================= PLAYER SLOT IMPLEMENTATION ================= */

void WaitingRoomState::PlayerSlot::setup(sf::Font& font) {
    // Thiết kế panel phẳng, bo góc nhẹ (thông qua cảm giác màu sắc)
    panelBg.setSize({220.f, 140.f});
    panelBg.setOrigin(110.f, 70.f);
    panelBg.setFillColor(sf::Color(45, 52, 54, 200)); // Màu xám đen sang trọng
    panelBg.setOutlineThickness(1.5f);
    panelBg.setOutlineColor(sf::Color(99, 110, 114));

    // Avatar tròn tối giản
    avatarBg.setRadius(32.f);
    avatarBg.setOrigin(32.f, 32.f);
    avatarBg.setOutlineThickness(2.f);
    avatarBg.setOutlineColor(sf::Color(255, 255, 255, 50));

    avatarLetter.setFont(font);
    avatarLetter.setCharacterSize(28);
    avatarLetter.setFillColor(sf::Color::White);

    nameText.setFont(font);
    nameText.setCharacterSize(17);
    nameText.setFillColor(sf::Color::White);

    balanceText.setFont(font);
    balanceText.setCharacterSize(14);
    balanceText.setFillColor(sf::Color(253, 203, 110)); // Màu vàng nhẹ

    readyStatusText.setFont(font);
    readyStatusText.setCharacterSize(13);
}

void WaitingRoomState::PlayerSlot::setContent(const RoomMember& member) {
    // Bảng màu Flat Design cho Avatar
    static sf::Color avColors[] = {
        sf::Color(9, 132, 227),  // Blue
        sf::Color(0, 184, 148),  // Green
        sf::Color(108, 92, 231), // Purple
        sf::Color(225, 112, 85)  // Orange
    };

    avatarBg.setFillColor(avColors[member.id % 4]);

    std::string initial = member.name.empty() ? "?" : utf8_first_char(member.name);
    avatarLetter.setString(toSf(initial));
    nameText.setString(toSf(member.name));
    balanceText.setString(toSf(formatMoney(member.balance)));

    if (member.isReady) {
        readyStatusText.setString(toSf("ĐÃ SẴN SÀNG"));
        readyStatusText.setFillColor(sf::Color(85, 239, 196));
        panelBg.setOutlineColor(sf::Color(0, 184, 148));
    } else {
        readyStatusText.setString(toSf("ĐANG CHỜ..."));
        readyStatusText.setFillColor(sf::Color(178, 190, 195));
        panelBg.setOutlineColor(sf::Color(99, 110, 114));
    }

    // Cập nhật vị trí các thành phần
    panelBg.setPosition(position);
    avatarBg.setPosition(position.x, position.y - 45.f);
    centerText(avatarLetter, avatarBg.getPosition().x, avatarBg.getPosition().y);
    centerText(nameText, position.x, position.y + 12.f);
    centerText(balanceText, position.x, position.y + 35.f);
    centerText(readyStatusText, position.x, position.y + 55.f);
}

void WaitingRoomState::PlayerSlot::setEmpty() {
    panelBg.setOutlineColor(sf::Color(99, 110, 114, 100));
    panelBg.setFillColor(sf::Color(45, 52, 54, 100));
    panelBg.setPosition(position);

    avatarBg.setFillColor(sf::Color(30, 30, 30, 150));
    avatarBg.setPosition(position.x, position.y - 45.f);

    avatarLetter.setString(toSf("+"));
    centerText(avatarLetter, avatarBg.getPosition().x, avatarBg.getPosition().y);

    nameText.setString(toSf("TRỐNG"));
    nameText.setFillColor(sf::Color(150, 150, 150));
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

/* ================= WAITING ROOM STATE IMPLEMENTATION ================= */

WaitingRoomState::WaitingRoomState(StateContext& ctx)
    : ctx_(ctx), isMyReady_(false) {

    background_.setSize({1280, 720});
    background_.setFillColor(sf::Color(30, 39, 46)); // Màu nền tối sang trọng

    headerBar_.setSize({1280, 56});
    headerBar_.setFillColor(sf::Color(0, 0, 0, 180));

    roomNameText_.setFont(ctx_.font);
    roomNameText_.setCharacterSize(24);
    roomNameText_.setFillColor(sf::Color::White);
    roomNameText_.setPosition(25, 12);

    subTitleText_.setFont(ctx_.font);
    subTitleText_.setCharacterSize(14);
    subTitleText_.setFillColor(sf::Color(178, 190, 195));
    subTitleText_.setPosition(400, 20);

    btnLeave_.setFont(ctx_.font);
    btnLeave_.setText(toSf("RỜI PHÒNG"), 16);
    btnLeave_.setSize({120, 36});
    btnLeave_.setPosition({1140, 10});
    btnLeave_.setColors(sf::Color(192, 57, 43), sf::Color::White, sf::Color::White);
    btnLeave_.setCallback([this]{ onLeaveClicked(); });

    btnReady_.setFont(ctx_.font);
    btnReady_.setText(toSf("SẴN SÀNG"), 22);
    btnReady_.setSize({220, 60});
    btnReady_.setPosition({640 - 110, 620});
    btnReady_.setColors(sf::Color(39, 174, 96), sf::Color::White, sf::Color::White);
    btnReady_.setCallback([this]{ onReadyClicked(); });

    btnChat_.setFont(ctx_.font);
    btnChat_.setText(toSf("CHAT"), 14);
    btnChat_.setSize({60, 60});
    btnChat_.setPosition({1190, 620});
    btnChat_.setColors(sf::Color(41, 128, 185), sf::Color::White, sf::Color::White);

    setupSlotsLayout();
}

void WaitingRoomState::setupSlotsLayout() {
    for (int i = 0; i < 4; ++i) slots_[i].setup(ctx_.font);
    // Bố cục bàn chơi hình thoi đối xứng
    slots_[0].position = {640.f, 480.f}; // Người chơi (dưới)
    slots_[1].position = {1000.f, 320.f}; // Phải
    slots_[2].position = {640.f, 160.f};  // Trên
    slots_[3].position = {280.f, 320.f};  // Trái
}

void WaitingRoomState::onEnter() {
    isMyReady_ = false;
    members_.clear();
    currentRoomInfo_ = ctx_.currentRoomInfo;

    roomNameText_.setString(toSf("PHÒNG: " + currentRoomInfo_.name));
    subTitleText_.setString(toSf("Mức cược: " + formatMoney(currentRoomInfo_.bet) + " | Chế độ: " + currentRoomInfo_.type));

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
}

void WaitingRoomState::onExit() {
    ctx_.roomHandler.setRoomUpdateCallback(nullptr);
    ctx_.roomHandler.setGameCountdownCallback(nullptr);
    ctx_.roomHandler.setGameStartCallback(nullptr);
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

    // Cập nhật trạng thái nút Ready của chính mình
    isMyReady_ = members_[myIndex].isReady;
    btnReady_.setText(toSf(isMyReady_ ? "UNReady" : "Ready"));
    btnReady_.setColors(isMyReady_ ? sf::Color(230, 126, 34) : sf::Color(39, 174, 96), sf::Color::White, sf::Color::White);

    // Xếp mình vào Slot 0 (Vị trí trung tâm dưới)
    slots_[0].setContent(members_[myIndex]);

    // Xếp những người khác vào các slot còn lại
    int uiSlot = 1;
    for (size_t i = 0; i < members_.size() && uiSlot < 4; ++i) {
        if ((int)i == myIndex) continue;
        slots_[uiSlot++].setContent(members_[i]);
    }
    // Các slot còn lại để trống
    for (; uiSlot < 4; ++uiSlot) slots_[uiSlot].setEmpty();
}

void WaitingRoomState::onReadyClicked() {
    isMyReady_ = !isMyReady_;
    ctx_.network.roomSender().sendReady(isMyReady_);
    // Cập nhật UI tạm thời để tạo cảm giác mượt mà (Optimistic UI)
    btnReady_.setText(toSf(isMyReady_ ? "HỦY SẴN SÀNG" : "SẴN SÀNG"));
    btnReady_.setColors(isMyReady_ ? sf::Color(230, 126, 34) : sf::Color(39, 174, 96), sf::Color::White, sf::Color::White);
}

void WaitingRoomState::onLeaveClicked() {
    ctx_.network.roomSender().sendLeaveRoom();
    ctx_.requestTransition(GameStateType::Lobby);
}

void WaitingRoomState::handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) {
    btnLeave_.handleEvent(e, mousePos);
    btnReady_.handleEvent(e, mousePos);
    btnChat_.handleEvent(e, mousePos);
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
}