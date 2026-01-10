#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <chrono>

#include "../../model/game_event.h"
#include "game_config.h"

#include "../../db/repository/player_repository.h"
#include "../../db/repository/card_repository.h"
#include "../../db/repository/game_repository.h"


enum class GameState {
    WAITING,
    BIDDING,
    PLAYING,
    FINISHED
};


struct Player {
    int id = -1;
    std::string name;
    bool isReady = false;
    bool isDisconnected = false;
    std::vector<DBCard> hand;
    long long balance = 0;
    bool hasPassed = false;
};

enum HandType {
    INVALID = 0,
    SINGLE = 1,      // Cóc
    PAIR = 2,        // Đôi
    TRIPLE = 3,      // Sám
    QUAD = 4,        // Tứ quý
    SEQUENCE = 5     // Sảnh (3 lá trở lên)
};

struct HandInfo {
    HandType type;
    int power;
    int count;
};

class SamLocGame {
public:
    SamLocGame(
        int rId,
        const DBRoom& info,
        const std::vector<DBCard>& deck,
        PlayerRepository& pRepo,
        GameRepository& gRepo
    );

    std::vector<GameEvent> addPlayer(int playerId);
    std::vector<GameEvent> removePlayer(int playerId);
    std::vector<GameEvent> onPlayerDisconnect(int playerId);

    std::vector<GameEvent> setPlayerReady(int playerId, bool ready);
    std::vector<GameEvent> playCards(int playerId, std::vector<int> cardIds);
    std::vector<GameEvent> passTurn(int playerId);

    std::vector<GameEvent> update();

    std::string getPlayersStateJsonFor(int viewerId);
    std::vector<int> getAllPlayerIds();
    bool isPlayerInGame(int playerId);

    int getCurrentPlayerCount() const { return (int)players.size(); }
    GameState getState() const { return state; }
    bool isJoinAllowed() const { return !lockedForJoin && state == GameState::WAITING; }
    bool canBeat(const std::vector<DBCard>& cardsToPlay, std::string& err);
    HandInfo analyzeHand(const std::vector<DBCard>& cards);
private:
    std::vector<GameEvent> startPlayingPhase(int firstPlayerId);
    std::vector<GameEvent> handleWin(int winnerId, int reasonCode);

    void nextTurnIndex();
    int pickFirstPlayerId();
    int countActivePlayers();

    bool isValidMove(const std::vector<DBCard>& cards, std::string& err);

    std::vector<GameEvent> broadcastMoveResult(
        int actorId,
        std::string action,
        std::vector<int> cards
    );

    int roomId;
    DBRoom roomInfo;
    std::vector<DBCard> masterDeck;

    PlayerRepository& playerRepo;
    GameRepository& gameRepo;

    std::vector<Player> players;
    GameState state = GameState::WAITING;

    int currentGameId = 0;
    int currentPlayerIndex = -1;
    int lastTurnOwnerId = -1;

    std::vector<int> boardCards;

    bool startingScheduled = false;
    bool lockedForJoin = false;

    std::chrono::steady_clock::time_point scheduledStartTime;
    std::chrono::steady_clock::time_point phaseStartTime;
};
