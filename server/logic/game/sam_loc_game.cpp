#include "sam_loc_game.h"
#include "../../utils/logger.h"
#include <nlohmann/json.hpp>
#include <random>
#include <algorithm>
#include <chrono>

using json = nlohmann::json;

SamLocGame::SamLocGame(int rId, const DBRoom& info, const std::vector<DBCard>& deck, PlayerRepository& pRepo, GameRepository& gRepo)
    : roomId(rId), roomInfo(info), masterDeck(deck), playerRepo(pRepo), gameRepo(gRepo) {
    state = GameState::WAITING;
    currentGameId = 0;
    currentPlayerIndex = -1;
    lastTurnOwnerId = -1;
    startingScheduled = false;
    lockedForJoin = false;
}

bool SamLocGame::isPlayerInGame(int playerId) {
    for (const auto& p : players) if (p.id == playerId) return true;
    return false;
}

std::vector<int> SamLocGame::getAllPlayerIds() {
    std::vector<int> ids;
    for (const auto& p : players) ids.push_back(p.id);
    return ids;
}

std::string SamLocGame::getPlayersStateJsonFor(int viewerId) {
    json jArr = json::array();
    for (auto& p : players) {
        json j;
        j["id"] = p.id;
        j["name"] = p.name;
        j["isReady"] = p.isReady;
        j["balance"] = p.balance;
        j["handSize"] = (int)p.hand.size();
        j["hasPassed"] = p.hasPassed;
        j["isDisconnected"] = p.isDisconnected;
        if (p.id == viewerId) {
            std::sort(p.hand.begin(), p.hand.end(), [](const DBCard& a, const DBCard& b) {
                return a.orderValue < b.orderValue;
            });
            json h = json::array();
            for (const auto& c : p.hand) h.push_back(c.id);
            j["hand"] = h;
        }
        jArr.push_back(j);
    }
    return jArr.dump();
}

int SamLocGame::countActivePlayers() {
    int count = 0;
    for (const auto& p : players) if (!p.isDisconnected) count++;
    return count;
}

HandInfo SamLocGame::analyzeHand(const std::vector<DBCard>& cards) {
    HandInfo info = { INVALID, 0, (int)cards.size() };
    if (cards.empty()) return info;
    std::vector<DBCard> sortedCards = cards;
    std::sort(sortedCards.begin(), sortedCards.end(), [](const DBCard& a, const DBCard& b) {
        return a.orderValue < b.orderValue;
    });
    std::vector<int> values;
    for (const auto& c : sortedCards) values.push_back(c.orderValue);
    int n = (int)values.size();
    info.power = values.back();
    if (n == 1) {
        info.type = SINGLE;
    } else if (n == 2 && values[0] == values[1]) {
        info.type = PAIR;
    } else if (n == 3 && values[0] == values[2]) {
        info.type = TRIPLE;
    } else if (n == 4 && values[0] == values[3]) {
        info.type = QUAD;
    } else if (n >= 3) {
        bool isSeq = true;
        for (int i = 0; i < n - 1; ++i) {
            if (values[i + 1] != values[i] + 1 || values[i + 1] == 15) {
                isSeq = false;
                break;
            }
        }
        if (isSeq) info.type = SEQUENCE;
    }
    return info;
}

bool SamLocGame::canBeat(const std::vector<DBCard>& cardsToPlay, std::string& err) {
    HandInfo newHand = analyzeHand(cardsToPlay);
    if (newHand.type == INVALID) {
        err = "Invalid hand type";
        return false;
    }
    if (boardCards.empty()) return true;
    std::vector<DBCard> currentBoard;
    for (int cid : boardCards) {
        for (const auto& mc : masterDeck) {
            if (mc.id == cid) {
                currentBoard.push_back(mc);
                break;
            }
        }
    }
    HandInfo boardHand = analyzeHand(currentBoard);
    if (boardHand.type == SINGLE && boardHand.power == 15 && newHand.type == QUAD) return true;
    if (newHand.type == boardHand.type && newHand.count == boardHand.count) {
        if (newHand.power > boardHand.power) return true;
        err = "Hand power is lower than board";
    } else {
        err = "Hand type mismatch";
    }
    return false;
}

std::vector<GameEvent> SamLocGame::broadcastMoveResult(int actorId, std::string action, std::vector<int> cards) {
    std::vector<GameEvent> events;
    int nextId = (state != GameState::PLAYING) ? -1 : players[currentPlayerIndex].id;
    auto now = std::chrono::steady_clock::now();
    int elapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(now - phaseStartTime).count();
    int remain = std::max(0, (int)GAME_TURN_TIMEOUT_SECONDS - elapsed);
    for (const auto& target : players) {
        json fatJson;
        fatJson["lastMove"] = {{"userId", actorId}, {"action", action}, {"cards", cards}};
        fatJson["boardCards"] = boardCards;
        fatJson["nextTurnId"] = nextId;
        fatJson["timeout"] = remain;
        json members = json::array();
        for (auto& m : players) {
            json mj;
            mj["id"] = m.id;
            mj["name"] = m.name;
            mj["handSize"] = (int)m.hand.size();
            mj["hasPassed"] = m.hasPassed;
            mj["isDisconnected"] = m.isDisconnected;
            mj["balance"] = m.balance;
            if (m.id == target.id) {
                std::vector<DBCard> sortedH = m.hand;
                std::sort(sortedH.begin(), sortedH.end(), [](const DBCard& a, const DBCard& b) {
                    return a.orderValue < b.orderValue;
                });
                json hj = json::array();
                for (const auto& c : sortedH) hj.push_back(c.id);
                mj["hand"] = hj;
            }
            members.push_back(mj);
        }
        fatJson["members"] = members;
        GameEvent ev;
        ev.type = MessageType::S_MOVE_RESULT;
        ev.payload = fatJson.dump();
        ev.targetPlayerIds = { target.id };
        ev.roomId = roomId;
        events.push_back(ev);
    }
    return events;
}

std::vector<GameEvent> SamLocGame::addPlayer(int playerId) {
    std::vector<GameEvent> events;
    DBPlayer dbp;
    if (!playerRepo.getPlayer(playerId, dbp)) return events;
    double minRequired = (roomInfo.type == "dem_la") ? (roomInfo.betAmount * 15.0) : (roomInfo.betAmount * 2.0);
    if (dbp.balance < minRequired) {
        GameEvent failEv;
        failEv.type = MessageType::S_JOIN_ROOM_FAIL;
        failEv.payload = json({{"success", false}, {"message", "Insufficient funds!"}}).dump();
        failEv.targetPlayerIds = { playerId };
        return {failEv};
    }
    if (players.size() >= (size_t)roomInfo.maxPlayers || lockedForJoin) {
        GameEvent failEv;
        failEv.type = MessageType::S_JOIN_ROOM_FAIL;
        failEv.payload = json({{"success", false}, {"message", "Room full or playing!"}}).dump();
        failEv.targetPlayerIds = { playerId };
        return {failEv};
    }
    if (isPlayerInGame(playerId)) return events;
    Player p;
    p.id = playerId;
    p.name = dbp.displayName;
    p.balance = dbp.balance;
    p.isReady = false;
    p.isDisconnected = false;
    p.hasPassed = false;
    players.push_back(p);
    GameEvent successEv;
    successEv.type = MessageType::S_JOIN_ROOM_SUCCESS;
    successEv.payload = json({
        {"success", true},
        {"roomId", roomId},
        {"roomInfo", {
            {"id", roomId},
            {"name", roomInfo.roomName},
            {"bet", (long long)roomInfo.betAmount},
            {"type", roomInfo.type}
        }}
    }).dump();
    successEv.targetPlayerIds = { playerId };
    events.push_back(successEv);
    GameEvent updateEv;
    updateEv.type = MessageType::S_ROOM_UPDATE;
    updateEv.payload = getPlayersStateJsonFor(-1);
    updateEv.targetPlayerIds = getAllPlayerIds();
    updateEv.roomId = roomId;
    events.push_back(updateEv);
    return events;
}

std::vector<GameEvent> SamLocGame::removePlayer(int playerId) {
    std::vector<GameEvent> events;
    auto it = std::remove_if(players.begin(), players.end(), [playerId](const Player& p) { 
        return p.id == playerId; 
    });
    if (it != players.end()) {
        players.erase(it, players.end());
        startingScheduled = false;
        std::vector<int> ids = getAllPlayerIds();
        GameEvent ev;
        ev.type = MessageType::S_ROOM_UPDATE;
        ev.payload = getPlayersStateJsonFor(-1);
        ev.targetPlayerIds = ids;
        ev.roomId = roomId;
        events.push_back(ev);
    }
    return events;
}

std::vector<GameEvent> SamLocGame::onPlayerDisconnect(int playerId) {
    if (state == GameState::WAITING || state == GameState::FINISHED) {
        return removePlayer(playerId);
    }
    std::vector<GameEvent> events;
    Player* pOut = nullptr;
    for (auto& p : players) if (p.id == playerId) pOut = &p;
    if (pOut && !pOut->isDisconnected) {
        pOut->isDisconnected = true;
        pOut->hasPassed = true;
        gameRepo.logMove(currentGameId, playerId, "DISCONNECT", "[]");
        json jMsg;
        jMsg["userId"] = playerId;
        jMsg["msg"] = "Player disconnected.";
        GameEvent discEv;
        discEv.type = MessageType::PLAYER_DISCONNECT_GAME;
        discEv.payload = jMsg.dump();
        discEv.targetPlayerIds = getAllPlayerIds();
        discEv.roomId = roomId;
        events.push_back(discEv);
        if (state == GameState::PLAYING && players[currentPlayerIndex].id == playerId) {
            auto passEvs = passTurn(playerId);
            events.insert(events.end(), passEvs.begin(), passEvs.end());
        }
        if (countActivePlayers() <= 1) {
            int winId = -1;
            for (const auto& p : players) if (!p.isDisconnected) winId = p.id;
            if (winId != -1) {
                auto winEvs = handleWin(winId, 2); 
                events.insert(events.end(), winEvs.begin(), winEvs.end());
            }
        }
    }
    return events;
}

std::vector<GameEvent> SamLocGame::setPlayerReady(int playerId, bool ready) {
    std::vector<GameEvent> events;
    if (state != GameState::WAITING && state != GameState::FINISHED) return events;
    if (state == GameState::FINISHED) state = GameState::WAITING;
    int rCount = 0;
    for (auto& p : players) {
        if (p.id == playerId) p.isReady = ready;
        if (p.isReady) ++rCount;
    }
    GameEvent evSync;
    evSync.type = MessageType::S_ROOM_UPDATE;
    evSync.payload = getPlayersStateJsonFor(-1);
    evSync.targetPlayerIds = getAllPlayerIds();
    evSync.roomId = roomId;
    events.push_back(evSync);
    if (players.size() >= 2 && (size_t)rCount == players.size()) {
        if (!startingScheduled) {
            startingScheduled = true;
            scheduledStartTime = std::chrono::steady_clock::now();
            json jS; jS["startIn"] = GAME_READY_DELAY_SECONDS;
            GameEvent evS;
            evS.type = MessageType::S_GAME_START_COUNTDOWN;
            evS.payload = jS.dump();
            evS.targetPlayerIds = getAllPlayerIds();
            evS.roomId = roomId;
            events.push_back(evS);
        }
    } else {
        startingScheduled = false;
    }
    return events;
}

std::vector<GameEvent> SamLocGame::startPlayingPhase(int firstId) {
    std::vector<GameEvent> events;
    if (players.empty()) return events;
    currentGameId = gameRepo.createGame(roomId, "SAM_LOC");
    state = GameState::PLAYING;
    lockedForJoin = true;
    std::vector<DBCard> deck = masterDeck;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
    int idx = 0;
    for (auto& p : players) {
        p.hand.clear();
        p.hasPassed = false;
        p.isReady = false;
        for (int i = 0; i < 10 && idx < (int)deck.size(); ++i) {
            p.hand.push_back(deck[idx++]);
        }
        std::sort(p.hand.begin(), p.hand.end(), [](const DBCard& a, const DBCard& b) {
            return a.orderValue < b.orderValue;
        });
        std::vector<int> hIds;
        for (const auto& card : p.hand) hIds.push_back(card.id);
        GameEvent ev;
        ev.type = MessageType::S_GAME_START;
        ev.payload = json({{"hand", hIds}, {"gameId", currentGameId}}).dump();
        ev.targetPlayerIds = { p.id };
        ev.roomId = roomId;
        events.push_back(ev);
    }
    currentPlayerIndex = 0;
    for (int i = 0; i < (int)players.size(); ++i) {
        if (players[i].id == firstId) { currentPlayerIndex = i; break; }
    }
    lastTurnOwnerId = -1;
    boardCards.clear();
    phaseStartTime = std::chrono::steady_clock::now();
    auto turnEvs = broadcastMoveResult(firstId, "start", {});
    events.insert(events.end(), turnEvs.begin(), turnEvs.end());
    return events;
}

std::vector<GameEvent> SamLocGame::playCards(int playerId, std::vector<int> cardIds) {
    std::vector<GameEvent> events;
    if (state != GameState::PLAYING) return events;
    if (currentPlayerIndex < 0 || players[currentPlayerIndex].id != playerId) return events;
    Player* p = &players[currentPlayerIndex];
    std::vector<DBCard> toPlay;
    for (int cid : cardIds) {
        auto it = std::find_if(p->hand.begin(), p->hand.end(), [cid](const DBCard& c) { return c.id == cid; });
        if (it != p->hand.end()) toPlay.push_back(*it);
    }
    std::string err;
    if (cardIds.empty() || toPlay.size() != cardIds.size() || !canBeat(toPlay, err)) {
        return broadcastMoveResult(playerId, "invalid", {});
    }
    gameRepo.logMove(currentGameId, playerId, "PLAY", json(cardIds).dump());
    for (int cid : cardIds) {
        p->hand.erase(std::remove_if(p->hand.begin(), p->hand.end(), [cid](const DBCard& c) { 
            return c.id == cid; 
        }), p->hand.end());
    }
    boardCards = cardIds;
    lastTurnOwnerId = playerId;
    if (p->hand.empty()) return handleWin(playerId, 1);
    nextTurnIndex();
    auto moveEvs = broadcastMoveResult(playerId, "play", cardIds);
    events.insert(events.end(), moveEvs.begin(), moveEvs.end());
    return events;
}

std::vector<GameEvent> SamLocGame::passTurn(int playerId) {
    std::vector<GameEvent> events;
    if (state != GameState::PLAYING) return events;
    if (currentPlayerIndex < 0 || players[currentPlayerIndex].id != playerId) return events;
    gameRepo.logMove(currentGameId, playerId, "PASS", "[]");
    players[currentPlayerIndex].hasPassed = true;
    nextTurnIndex();
    auto moveEvs = broadcastMoveResult(playerId, "pass", {});
    events.insert(events.end(), moveEvs.begin(), moveEvs.end());
    return events;
}

void SamLocGame::nextTurnIndex() {
    int next = currentPlayerIndex;
    int size = (int)players.size();
    int loop = 0;
    do {
        next = (next + 1) % size;
        loop++;
    } while ((players[next].hasPassed || players[next].isDisconnected) && loop < size);
    if (loop >= size || players[next].id == lastTurnOwnerId) {
        for (auto& p : players) if (!p.isDisconnected) p.hasPassed = false;
        boardCards.clear();
        lastTurnOwnerId = -1;
        if (loop >= size) {
            for (int i = 0; i < size; ++i) if (!players[i].isDisconnected) { next = i; break; }
        }
    }
    currentPlayerIndex = next;
    phaseStartTime = std::chrono::steady_clock::now();
}

std::vector<GameEvent> SamLocGame::handleWin(int winId, int reason) {
    state = GameState::FINISHED;
    lockedForJoin = false;
    gameRepo.endGame(currentGameId, winId);
    json losersArr = json::array();
    double totalWinPool = 0;
    Player* winnerPlayer = nullptr;
    std::vector<int> recipients = getAllPlayerIds(); 

    for (auto& p : players) {
        if (p.id == winId) { 
            winnerPlayer = &p; 
            continue; 
        }
        double mainPenalty = (roomInfo.type == "dem_la") ? (double)p.hand.size() * roomInfo.betAmount : roomInfo.betAmount;
        double quitPenalty = (reason == 2 && p.isDisconnected) ? roomInfo.betAmount * 1.0 : 0;
        double totalLost = mainPenalty + quitPenalty;
        totalWinPool += totalLost;
        p.balance -= totalLost;
        playerRepo.updateBalance(p.id, -totalLost);
        gameRepo.saveResult(currentGameId, p.id, 2, (int)p.hand.size(), -totalLost);
        
        json lO;
        lO["userId"] = p.id; lO["name"] = p.name; lO["cardsLeft"] = (int)p.hand.size();
        lO["mainPenalty"] = mainPenalty; lO["quitPenalty"] = quitPenalty;
        lO["totalChange"] = -totalLost; lO["isQuit"] = p.isDisconnected;
        losersArr.push_back(lO);
    }
    if (winnerPlayer) {
        winnerPlayer->balance += totalWinPool;
        playerRepo.updateBalance(winId, totalWinPool);
        gameRepo.saveResult(currentGameId, winId, 1, 0, totalWinPool);
    }

    json fP;
    fP["gameId"] = currentGameId;
    fP["winner"] = {{"id", winId}, {"name", winnerPlayer ? winnerPlayer->name : ""}, {"totalBonus", totalWinPool}};
    fP["losers"] = losersArr;
    fP["reason"] = reason;
    
    GameEvent ev;
    ev.type = MessageType::S_GAME_END;
    ev.payload = fP.dump();
    ev.targetPlayerIds = recipients;
    ev.roomId = roomId;

    players.clear();

    startingScheduled = false;
    boardCards.clear();
    currentPlayerIndex = -1;
    lastTurnOwnerId = -1;

    GameEvent syncEv;
    syncEv.type = MessageType::S_ROOM_UPDATE;
    syncEv.payload = "[]"; 
    syncEv.targetPlayerIds = recipients;
    syncEv.roomId = roomId;
    
    return {ev, syncEv};
}

int SamLocGame::pickFirstPlayerId() {
    std::vector<int> ids;
    for (auto& p : players) if (!p.isDisconnected) ids.push_back(p.id);
    if (ids.empty()) return -1;
    std::random_device rd; std::mt19937 g(rd());
    std::uniform_int_distribution<> d(0, (int)ids.size() - 1);
    return ids[d(g)];
}

std::vector<GameEvent> SamLocGame::update() {
    std::vector<GameEvent> events;
    auto now = std::chrono::steady_clock::now();
    if (state == GameState::WAITING && startingScheduled) {
        auto elap = std::chrono::duration_cast<std::chrono::seconds>(now - scheduledStartTime).count();
        if (elap >= GAME_READY_DELAY_SECONDS) {
            startingScheduled = false;
            int fId = pickFirstPlayerId();
            if (fId != -1) {
                auto evs = startPlayingPhase(fId);
                events.insert(events.end(), evs.begin(), evs.end());
            }
        }
    }
    if (state == GameState::PLAYING) {
        auto elap = std::chrono::duration_cast<std::chrono::seconds>(now - phaseStartTime).count();
        if (elap > GAME_TURN_TIMEOUT_SECONDS) {
            if (currentPlayerIndex >= 0) {
                auto evs = passTurn(players[currentPlayerIndex].id);
                events.insert(events.end(), evs.begin(), evs.end());
            }
        }
    }
    return events;
}