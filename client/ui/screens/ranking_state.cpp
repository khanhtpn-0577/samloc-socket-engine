#include "ranking_state.h"
#include <iostream>
#include <algorithm>

RankingState::RankingState(StateContext& ctx)
    : ctx_(ctx) {

    // ===== Background =====
    background_.setSize({1280.f, 720.f});
    background_.setFillColor(sf::Color(25, 25, 40));

    // ===== Main Panel =====
    panel_.setSize({900.f, 520.f});
    panel_.setPosition({190.f, 120.f});
    panel_.setFillColor(sf::Color(40, 40, 70));
    panel_.setOutlineThickness(4.f);
    panel_.setOutlineColor(sf::Color(120, 120, 200));

    // ===== Title =====
    titleText_.setFont(ctx_.font);
    titleText_.setString("Friends Ranking");
    titleText_.setCharacterSize(36);
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setPosition(420.f, 140.f);

    // ===== Header =====
    headerRank_.setFont(ctx_.font);
    headerRank_.setCharacterSize(22);
    headerRank_.setFillColor(sf::Color(200, 200, 200));
    headerRank_.setString("Rank");
    headerRank_.setPosition(colRankX_, 210.f);

    headerName_.setFont(ctx_.font);
    headerName_.setCharacterSize(22);
    headerName_.setFillColor(sf::Color(200, 200, 200));
    headerName_.setString("Player");
    headerName_.setPosition(colNameX_, 210.f);

    headerScore_.setFont(ctx_.font);
    headerScore_.setCharacterSize(22);
    headerScore_.setFillColor(sf::Color(200, 200, 200));
    headerScore_.setString("Balance");
    headerScore_.setPosition(colScoreX_, 210.f);

    // ===== Back Button =====
    backButton_.setFont(ctx_.font);
    backButton_.setText("Back to Lobby", 20);
    backButton_.setSize({200.f, 45.f});
    backButton_.setPosition({540.f, 600.f});
    backButton_.setColors(
        sf::Color(80, 80, 120),
        sf::Color::White,
        sf::Color::White
    );
    backButton_.setCallback([this]() {
        ctx_.requestTransition(GameStateType::Lobby);
    });

    // ===== Pagination Buttons =====
    prevPageButton_.setFont(ctx_.font);
    prevPageButton_.setText("< Prev", 18);
    prevPageButton_.setSize({120.f, 40.f});
    prevPageButton_.setPosition({360.f, 600.f});
    prevPageButton_.setColors(
        sf::Color(70, 70, 110),     // background
        sf::Color::White,           // text
        sf::Color(150, 150, 220)    // hover
    );
    prevPageButton_.setCallback([this]() {
        if (currentPage_ > 0) {
            currentPage_--;
            buildRankingTexts();
        }
    });


    nextPageButton_.setFont(ctx_.font);
    nextPageButton_.setText("Next >", 18);
    nextPageButton_.setSize({120.f, 40.f});
    nextPageButton_.setPosition({800.f, 600.f});
    nextPageButton_.setColors(
        sf::Color(70, 70, 110),     // background
        sf::Color::White,           // text
        sf::Color(150, 150, 220)    // hover
    );
    nextPageButton_.setCallback([this]() {
        int maxPage = (rankings_.size() - 1) / PAGE_SIZE;
        if (currentPage_ < maxPage) {
            currentPage_++;
            buildRankingTexts();
        }
    });


    buildDummyData();
    buildRankingTexts();
}

// ================= DATA =================

void RankingState::buildDummyData() {
    rankings_ = {
        {1, "Alice",   1200000},
        {2, "Bob",      950000},
        {3, "Charlie",  720000},
        {4, "David",    500000},
        {5, "You",      static_cast<long long>(ctx_.session.balance())},
        {6, "Eve",      420000},
        {7, "Frank",    380000}
    };
}

// ================= TABLE =================

void RankingState::buildRankingTexts() {
    rankingTexts_.clear();
    tableLines_.clear();

    int start = currentPage_ * PAGE_SIZE;
    int end   = std::min(start + PAGE_SIZE, (int)rankings_.size());

    float y = tableStartY_;

    // Vertical lines
    auto vLine = [&](float x) {
        sf::RectangleShape l;
        l.setPosition(x, tableStartY_ - 10.f);
        l.setSize({1.f, rowHeight_ * PAGE_SIZE + 10.f});
        l.setFillColor(sf::Color(100, 100, 140));
        tableLines_.push_back(l);
    };

    vLine(colRankX_ - 10.f);
    vLine(colNameX_ - 10.f);
    vLine(colScoreX_ - 10.f);
    vLine(colScoreX_ + 130.f);

    for (int i = start; i < end; ++i) {
        const auto& e = rankings_[i];

        sf::Text rank, name, score;

        rank.setFont(ctx_.font);
        rank.setCharacterSize(20);
        rank.setFillColor(sf::Color::White);
        rank.setString(std::to_string(e.rank));
        rank.setPosition(colRankX_, y);

        name.setFont(ctx_.font);
        name.setCharacterSize(20);
        name.setFillColor(sf::Color::White);
        name.setString(e.name);
        name.setPosition(colNameX_, y);

        score.setFont(ctx_.font);
        score.setCharacterSize(20);
        score.setFillColor(sf::Color(255, 215, 0));
        score.setString(std::to_string(e.score));
        score.setPosition(colScoreX_, y);

        rankingTexts_.push_back(rank);
        rankingTexts_.push_back(name);
        rankingTexts_.push_back(score);

        // Horizontal line
        sf::RectangleShape h;
        h.setPosition(tableStartX_, y + rowHeight_ - 8.f);
        h.setSize({620.f, 1.f});
        h.setFillColor(sf::Color(90, 90, 130));
        tableLines_.push_back(h);

        y += rowHeight_;
    }
}

// ================= NETWORK =================

void RankingState::requestFriendRanking() {
    ctx_.rankHandler.requestFriendRank();
}

// ================= LIFECYCLE =================

void RankingState::onEnter() {
    std::cout << "[RankingState] Entered\n";

    ctx_.rankHandler.setFriendRankCallback(
        [this](const std::vector<FriendRankInfo>& ranks) {
            rankings_.clear();
            int r = 1;
            for (const auto& f : ranks) {
                rankings_.push_back({r++, f.name, f.balance});
            }
            currentPage_ = 0;
            buildRankingTexts();
        }
    );

    requestFriendRanking();
}

void RankingState::onExit() {
    std::cout << "[RankingState] Exited\n";
}

// ================= LOOP =================

void RankingState::handleEvent(const sf::Event& event,
                               const sf::Vector2f& mousePos) {
    backButton_.handleEvent(event, mousePos);
    prevPageButton_.handleEvent(event, mousePos);
    nextPageButton_.handleEvent(event, mousePos);
}

void RankingState::update(float dt) {
    (void)dt;
}

void RankingState::draw(sf::RenderWindow& window) {
    window.draw(background_);
    window.draw(panel_);
    window.draw(titleText_);

    window.draw(headerRank_);
    window.draw(headerName_);
    window.draw(headerScore_);

    for (auto& l : tableLines_) window.draw(l);
    for (auto& t : rankingTexts_) window.draw(t);

    backButton_.draw(window);
    prevPageButton_.draw(window);
    nextPageButton_.draw(window);
}
