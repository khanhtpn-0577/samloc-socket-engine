#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <chrono>
#include <sstream>
#include <algorithm>

#include "../../model/game_event.h" 
#include <nlohmann/json.hpp>
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
    int id;
    std::string name;
    bool isReady;
    bool isDisconnected;
    std::vector<DBCard> hand;
    bool wantsSam;
    bool hasPassed;
};

struct MoveInfo {
    int type;
    int highestRank;
    int count; 
};

class SamLocGame {
public:
    SamLocGame(int rId, const DBRoom& info, const std::vector<DBCard>& deck,
               PlayerRepository& pRepo, GameRepository& gRepo)
        : roomId(rId), roomInfo(info), masterDeck(deck), 
          playerRepo(pRepo), gameRepo(gRepo) {
        state = GameState::WAITING;
        currentPlayerIndex = -1;
        currentGameId = 0;
        startingScheduled = false;
        lockedForJoin = false;
    }

    // --- Core Actions ---
    std::vector<GameEvent> addPlayer(int playerId, const std::string& name);
    std::vector<GameEvent> removePlayer(int playerId); // Bypass Room
    std::vector<GameEvent> onPlayerDisconnect(int playerId);
    
    // --- User Actions ---
    std::vector<GameEvent> setPlayerReady(int playerId, bool ready);
    std::vector<GameEvent> handleBaoSam(int playerId, bool wantSam);
    std::vector<GameEvent> playCards(int playerId, std::vector<int> cardIds);
    std::vector<GameEvent> passTurn(int playerId);

    // --- Loop ---
    std::vector<GameEvent> update();

    // --- Getters ---
    std::string getPlayersStateJson();
    int getCurrentPlayerCount() const { return (int)players.size(); }
    GameState getState() const { return state; } 

    bool isJoinAllowed() const { return !lockedForJoin && state == GameState::WAITING; }
    std::vector<int> getAllPlayerIds();
    bool isPlayerInGame(int playerId);

private:
    std::vector<GameEvent> startBiddingPhase();
    std::vector<GameEvent> endBiddingPhase(int samWinnerId);
    std::vector<GameEvent> startPlayingPhase(int firstPlayerId);
    std::vector<GameEvent> nextTurn();
    std::vector<GameEvent> handleWin(int winnerId, int reasonCode);
    
    int countActivePlayers();
    std::string vecToString(const std::vector<int>& v);
    bool isValidMove(const std::vector<DBCard>& cards, std::string& err);
    MoveInfo analyzeHand(const std::vector<DBCard>& cards);

    int roomId;
    DBRoom roomInfo;
    std::vector<DBCard> masterDeck;
    
    PlayerRepository& playerRepo;
    GameRepository& gameRepo;

    std::vector<Player> players;
    GameState state;
    int currentGameId;
    int currentPlayerIndex;
    
    bool startingScheduled;
    std::chrono::steady_clock::time_point scheduledStartTime;
    std::chrono::steady_clock::time_point phaseStartTime;
    
    bool lockedForJoin;
    std::unordered_set<int> biddingPlayers;
    int lastTurnOwnerId;
    MoveInfo lastMove;
};