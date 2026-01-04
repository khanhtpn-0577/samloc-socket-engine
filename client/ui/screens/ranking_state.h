#pragma once

#include "../game_state.h"
#include "../state_context.h"
#include "../components/button.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "../../handlers/rank/rank_handler.h"

// ================= DATA =================

struct RankingEntry {
    int rank;
    std::string name;
    long long score;
};

// ================= STATE =================

class RankingState : public GameState {
public:
    explicit RankingState(StateContext& ctx);

    void onEnter() override;
    void onExit() override;
    void handleEvent(const sf::Event& event,
                     const sf::Vector2f& mousePos) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

private:
    // ===== Context =====
    StateContext& ctx_;

    // ===== Background / Panel =====
    sf::RectangleShape background_;
    sf::RectangleShape panel_;

    // ===== Title =====
    sf::Text titleText_;

    // ===== Table Headers =====
    sf::Text headerRank_;
    sf::Text headerName_;
    sf::Text headerScore_;

    // ===== Ranking Data =====
    std::vector<RankingEntry> rankings_;
    std::vector<sf::Text> rankingTexts_;

    // ===== Table Layout =====
    float tableStartX_ = 260.f;
    float tableStartY_ = 240.f;
    float rowHeight_   = 42.f;

    float colRankX_  = 260.f;
    float colNameX_  = 340.f;
    float colScoreX_ = 720.f;

    // ===== Pagination =====
    int currentPage_ = 0;
    static constexpr int PAGE_SIZE = 5;

    // ===== Table Lines =====
    std::vector<sf::RectangleShape> tableLines_;

    // ===== Buttons =====
    Button backButton_;
    Button prevPageButton_;
    Button nextPageButton_;

    // ===== Helpers =====
    void buildDummyData();   // dùng khi chưa có server
    void buildRankingTexts();
    void requestFriendRanking();
};
