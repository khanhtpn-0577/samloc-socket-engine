#include "game_result_state.h"
#include <iostream>

GameResultState::GameResultState(StateContext& ctx) : ctx_(ctx) {
    setupLayout();
}

void GameResultState::setupLayout() {
    background_.setSize({1280.f, 720.f});
    background_.setFillColor(sf::Color(10, 45, 30, 230));

    panelBg_.setSize({1000.f, 480.f});
    panelBg_.setOrigin(500.f, 240.f);
    panelBg_.setPosition(640.f, 320.f);
    panelBg_.setFillColor(sf::Color(45, 30, 15, 255));
    panelBg_.setOutlineThickness(3.f);
    panelBg_.setOutlineColor(sf::Color(255, 215, 0));

    titleText_.setFont(ctx_.font);
    titleText_.setCharacterSize(55);
    titleText_.setStyle(sf::Text::Bold);
    titleText_.setFillColor(sf::Color(255, 215, 0));

    float btnWidth = 240.f;
    float btnHeight = 65.f;
    btnBack_.setFont(ctx_.font);
    btnBack_.setText("CONTINUE", 24);
    btnBack_.setSize({btnWidth, btnHeight});
    
    btnBack_.setPosition({640.f, 630.f}); 
    
    btnBack_.setColors(sf::Color(255, 215, 0), sf::Color::White, sf::Color::Black);
    btnBack_.setCallback([this]() {
        ctx_.requestTransition(GameStateType::RoomList);
    });
}

sf::String GameResultState::toSfString(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

std::string GameResultState::formatMoney(double value) {
    long long val = static_cast<long long>(value);
    std::string s = std::to_string(std::abs(val));
    int n = s.length() - 3;
    while (n > 0) { s.insert(n, "."); n -= 3; }
    return (val >= 0 ? "+" : "-") + s + " $";
}

void GameResultState::onEnter() {
    rows_.clear();
    const auto& j = ctx_.lastGameResult;

    titleText_.setString("GAME RESULT");
    sf::FloatRect tRect = titleText_.getLocalBounds();
    titleText_.setOrigin(tRect.left + tRect.width / 2.f, 0);
    titleText_.setPosition(640.f, 85.f);

    float startY = 205.f;

    if (j.contains("winner") && !j["winner"].is_null()) {
        auto w = j["winner"];
        PlayerRow r;
        r.bar.setSize({960.f, 55.f});
        r.bar.setPosition(160.f, startY - 5.f);
        r.bar.setFillColor(sf::Color(255, 215, 0, 80));

        r.name.setFont(ctx_.font);
        r.name.setCharacterSize(22);
        r.name.setString(toSfString(w.value("name", "")));
        r.name.setPosition(180.f, startY);
        r.name.setFillColor(sf::Color::White);

        r.status.setFont(ctx_.font);
        r.status.setCharacterSize(20);
        r.status.setString("WINNER");
        r.status.setPosition(480.f, startY);
        r.status.setFillColor(sf::Color(255, 215, 0));

        r.penaltyDetail.setFont(ctx_.font);
        r.penaltyDetail.setCharacterSize(18);
        r.penaltyDetail.setString("No Penalty");
        r.penaltyDetail.setPosition(680.f, startY);
        r.penaltyDetail.setFillColor(sf::Color(0, 191, 255));

        r.totalAmount.setFont(ctx_.font);
        r.totalAmount.setCharacterSize(24);
        r.totalAmount.setString(toSfString(formatMoney(w.value("totalBonus", 0.0))));
        r.totalAmount.setPosition(1100.f, startY);
        r.totalAmount.setFillColor(sf::Color::White);
        
        sf::FloatRect aRect = r.totalAmount.getLocalBounds();
        r.totalAmount.setOrigin(aRect.width, 0);
        rows_.push_back(r);
        startY += 65.f;
    }

    if (j.contains("losers") && j["losers"].is_array()) {
        for (const auto& l : j["losers"]) {
            PlayerRow r;
            r.bar.setSize({960.f, 55.f});
            r.bar.setPosition(160.f, startY - 5.f);
            r.bar.setFillColor(sf::Color(0, 0, 0, 150));

            r.name.setFont(ctx_.font);
            r.name.setCharacterSize(22);
            r.name.setString(toSfString(l.value("name", "")));
            r.name.setPosition(180.f, startY);
            r.name.setFillColor(sf::Color::White);

            std::string st = l.value("isQuit", false) ? "QUIT" : std::to_string(l.value("cardsLeft", 0)) + " cards";
            r.status.setFont(ctx_.font);
            r.status.setCharacterSize(20);
            r.status.setString(toSfString(st));
            r.status.setPosition(480.f, startY);
            r.status.setFillColor(sf::Color(0, 191, 255));

            double mainP = l.value("mainPenalty", 0.0);
            double quitP = l.value("quitPenalty", 0.0);
            std::string pStr = "-" + std::to_string((int)mainP/1000) + "k";
            if (quitP > 0) pStr += " / -" + std::to_string((int)quitP/1000) + "k";

            r.penaltyDetail.setFont(ctx_.font);
            r.penaltyDetail.setCharacterSize(18);
            r.penaltyDetail.setString(toSfString(pStr));
            r.penaltyDetail.setPosition(680.f, startY);
            r.penaltyDetail.setFillColor(sf::Color(220, 50, 50));

            r.totalAmount.setFont(ctx_.font);
            r.totalAmount.setCharacterSize(24);
            r.totalAmount.setString(toSfString(formatMoney(l.value("totalChange", 0.0))));
            r.totalAmount.setPosition(1100.f, startY);
            r.totalAmount.setFillColor(sf::Color(220, 50, 50));

            sf::FloatRect aRect = r.totalAmount.getLocalBounds();
            r.totalAmount.setOrigin(aRect.width, 0);
            rows_.push_back(r);
            startY += 60.f;
        }
    }
}

void GameResultState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(panelBg_);
    window.draw(titleText_);

    sf::Text h;
    h.setFont(ctx_.font);
    h.setCharacterSize(16);
    h.setFillColor(sf::Color(255, 215, 0, 180));
    
    h.setString("PLAYER"); h.setPosition(180.f, 175.f); window.draw(h);
    h.setString("STATUS"); h.setPosition(480.f, 175.f); window.draw(h);
    h.setString("PENALTY"); h.setPosition(680.f, 175.f); window.draw(h);
    h.setString("TOTAL"); h.setPosition(1040.f, 175.f); window.draw(h);

    for (auto& row : rows_) {
        window.draw(row.bar);
        window.draw(row.name);
        window.draw(row.status);
        window.draw(row.penaltyDetail);
        window.draw(row.totalAmount);
    }
    btnBack_.draw(window);
}

void GameResultState::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    btnBack_.handleEvent(event, mousePos);
}

void GameResultState::update(float dt) {}
void GameResultState::onExit() {}