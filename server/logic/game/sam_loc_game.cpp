#include "sam_loc_game.h"
#include "../../utils/logger.h"
#include <random>
#include <iostream>
#include <cmath>
#include <algorithm>

using json = nlohmann::json;

// =================================================================================
// HELPER FUNCTIONS
// =================================================================================

bool SamLocGame::isPlayerInGame(int playerId) {
    for (const auto& p : players) if (p.id == playerId) return true;
    return false;
}

std::vector<int> SamLocGame::getAllPlayerIds() {
    std::vector<int> ids;
    for (const auto& p : players) ids.push_back(p.id);
    return ids;
}

std::string SamLocGame::getPlayersStateJson() {
    json jArr = json::array();
    for (const auto& p : players) {
        jArr.push_back({
            {"id", p.id},
            {"name", p.name},
            {"isReady", p.isReady},
            {"isDisconnected", p.isDisconnected},
            {"wantsSam", p.wantsSam}
        });
    }
    return jArr.dump();
}

std::string SamLocGame::vecToString(const std::vector<int>& v) {
    std::stringstream ss; ss << "[";
    for (size_t i = 0; i < v.size(); ++i) ss << v[i] << (i < v.size() - 1 ? "," : "");
    ss << "]"; return ss.str();
}

int SamLocGame::countActivePlayers() {
    int count = 0; for(const auto& p: players) if(!p.isDisconnected) count++; return count;
}

// Dummy check: Bạn cần logic thật để so sánh bài
// Ở đây giả định bài đánh ra luôn hợp lệ về mặt cấu trúc, chỉ check logic chặn
MoveInfo SamLocGame::analyzeHand(const std::vector<DBCard>& cards) { 
    // Giả sử: return {Type, MaxRank, Count}
    // Cần logic thật để xác định đôi, sảnh...
    int maxRank = 0;
    for(auto& c : cards) if(c.orderValue > maxRank) maxRank = c.orderValue;
    return {1, maxRank, (int)cards.size()}; 
}

bool SamLocGame::isValidMove(const std::vector<DBCard>& cards, std::string& err) {
    if(cards.empty()){err="Chua chon bai";return false;}
    return true; 
}

// =================================================================================
// CORE ACTIONS
// =================================================================================

std::vector<GameEvent> SamLocGame::addPlayer(int playerId, const std::string& name) {
    std::vector<GameEvent> events;
    if (players.size() >= (size_t)roomInfo.maxPlayers) return events;
    for(const auto& p : players) if(p.id == playerId) return events;

    Player p; p.id = playerId; p.name = name; p.isReady = false;
    p.isDisconnected = false; p.hasPassed = false; p.wantsSam = false;
    players.push_back(p);

    Logger::log(LogLevel::INFO, "SAMLOC", "Player " + std::to_string(playerId) + " joined " + std::to_string(roomId));

    json jPayload; jPayload["id"] = p.id; jPayload["name"] = p.name; jPayload["isReady"] = false;
    GameEvent ev; ev.type = MessageType::S_PLAYER_JOINED; ev.payload = jPayload.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId;
    events.push_back(ev);
    return events;
}

std::vector<GameEvent> SamLocGame::removePlayer(int playerId) {
    std::vector<GameEvent> events;
    if (state != GameState::WAITING) return events;

    auto it = std::remove_if(players.begin(), players.end(), [playerId](const Player& p){ return p.id == playerId; });
    if (it != players.end()) {
        players.erase(it, players.end());
        std::vector<int> remaining = getAllPlayerIds();
        if (!remaining.empty()) {
            json jLeft; jLeft["userId"] = playerId; jLeft["msg"] = "Player left";
            GameEvent evLeft; evLeft.type = MessageType::S_PLAYER_LEFT; evLeft.payload = jLeft.dump();
            evLeft.targetPlayerIds = remaining; evLeft.roomId = roomId; events.push_back(evLeft);

            GameEvent evUpd; evUpd.type = MessageType::S_ROOM_UPDATE; evUpd.payload = getPlayersStateJson();
            evUpd.targetPlayerIds = remaining; evUpd.roomId = roomId; events.push_back(evUpd);
        }
        if (startingScheduled) {
            int rCount = 0; for(const auto& p : players) if(p.isReady) rCount++;
            if (players.size() < 2 || rCount < (int)players.size()) {
                startingScheduled = false;
                json jC; jC["event"] = "cancel_start"; jC["roomId"] = roomId;
                GameEvent evC; evC.type = MessageType::S_ROOM_UPDATE; evC.payload = jC.dump();
                evC.targetPlayerIds = remaining; evC.roomId = roomId; events.push_back(evC);
            }
        }
    }
    return events;
}

std::vector<GameEvent> SamLocGame::onPlayerDisconnect(int playerId) {
    if (state == GameState::WAITING) return removePlayer(playerId);

    std::vector<GameEvent> events;
    Player* pOut = nullptr;
    for (auto& p : players) if (p.id == playerId) pOut = &p;

    if (pOut && !pOut->isDisconnected) {
        Logger::log(LogLevel::INFO, "SAMLOC", "Player " + std::to_string(playerId) + " DISCONNECTED.");
        pOut->isDisconnected = true; pOut->hasPassed = true; // Auto pass

        json jOut; jOut["userId"] = playerId; jOut["msg"] = "Mat ket noi.";
        GameEvent ev; ev.type = MessageType::PLAYER_DISCONNECT_GAME; ev.payload = jOut.dump();
        ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);

        // Check < 2 người
        if (countActivePlayers() < 2) {
            int winnerId = -1; for(const auto& p:players) if(!p.isDisconnected) winnerId=p.id;
            if (winnerId!=-1) {
                // Thắng do đối thủ out
                auto evs = handleWin(winnerId, 2); 
                events.insert(events.end(), evs.begin(), evs.end());
                return events;
            }
        }

        // Logic Đền Sâm khi Disconnect
        if (state == GameState::PLAYING && pOut->wantsSam) {
             // Ông báo Sâm mà out -> Xử thua Đền Sâm luôn
             // Ở đây winnerId = playerId nghĩa là người này bị phạt (Logic handleWin sẽ xử lý)
             auto loseEvs = handleWin(playerId, 4); 
             events.insert(events.end(), loseEvs.begin(), loseEvs.end());
        } else if (state == GameState::PLAYING && !players.empty()) {
            if (players[currentPlayerIndex].id == playerId) {
                 auto passEv = passTurn(playerId);
                 events.insert(events.end(), passEv.begin(), passEv.end());
            }
        }
    }
    return events;
}

// =================================================================================
// GAME FLOW
// =================================================================================

std::vector<GameEvent> SamLocGame::setPlayerReady(int playerId, bool ready) {
    std::vector<GameEvent> events;
    if (state != GameState::WAITING) return events;
    
    bool found = false; int rCount = 0;
    for (auto& p : players) { if (p.id == playerId) { p.isReady = ready; found = true; } if (p.isReady) ++rCount; }
    if (!found) return events;

    json jReady; jReady["userId"] = playerId; jReady["isReady"] = ready;
    GameEvent ev; ev.type = MessageType::S_PLAYER_READY; ev.payload = jReady.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);

    if (players.size() >= 2 && (size_t)rCount == players.size()) {
        if (!startingScheduled) {
            startingScheduled = true;
            scheduledStartTime = std::chrono::steady_clock::now() + std::chrono::seconds(GAME_READY_DELAY_SECONDS);
            json jS; jS["event"] = "starting"; jS["startIn"] = GAME_READY_DELAY_SECONDS; jS["roomId"] = roomId;
            GameEvent evS; evS.type = MessageType::S_ROOM_UPDATE; evS.payload = jS.dump();
            evS.targetPlayerIds = getAllPlayerIds(); evS.roomId = roomId; events.push_back(evS);
        }
    } else {
        if (startingScheduled) {
            startingScheduled = false;
            json jC; jC["event"] = "cancel_start"; jC["roomId"] = roomId;
            GameEvent evC; evC.type = MessageType::S_ROOM_UPDATE; evC.payload = jC.dump();
            evC.targetPlayerIds = getAllPlayerIds(); evC.roomId = roomId; events.push_back(evC);
        }
    }
    return events;
}

std::vector<GameEvent> SamLocGame::startBiddingPhase() {
    std::vector<GameEvent> events;
    lockedForJoin = true; startingScheduled = false; state = GameState::BIDDING;
    try { currentGameId = gameRepo.createGame(roomId, roomInfo.type); } catch (...) {}
    
    std::vector<DBCard> deck = masterDeck; 
    std::random_device rd; std::mt19937 g(rd()); std::shuffle(deck.begin(), deck.end(), g);
    biddingPlayers.clear(); int instantWinId = -1;

    for (size_t i = 0; i < players.size(); ++i) {
        players[i].hand.clear(); players[i].hasPassed = false; players[i].wantsSam = false;
        int count2 = 0; json jHand = json::array();
        for (int k = 0; k < 10; ++k) {
            if (deck.empty()) break;
            DBCard c = deck.back(); deck.pop_back(); players[i].hand.push_back(c);
            if (c.rank == "2") ++count2; jHand.push_back(c.id);
        }
        if (count2 == 4) instantWinId = players[i].id;
        json jM; jM["gameId"] = currentGameId; jM["hand"] = jHand;
        GameEvent evD; evD.type = MessageType::S_GAME_START; evD.payload = jM.dump();
        evD.targetPlayerIds = { players[i].id }; evD.roomId = roomId; events.push_back(evD);
    }
    if (instantWinId != -1) return handleWin(instantWinId, 3);

    phaseStartTime = std::chrono::steady_clock::now();
    json jAsk; jAsk["timeout"] = GAME_BIDDING_TIMEOUT_SECONDS;
    GameEvent evA; evA.type = MessageType::S_ASK_BAO_SAM; evA.payload = jAsk.dump();
    evA.targetPlayerIds = getAllPlayerIds(); evA.roomId = roomId; events.push_back(evA);
    return events;
}

std::vector<GameEvent> SamLocGame::handleBaoSam(int playerId, bool wantSam) {
    std::vector<GameEvent> events;
    if (state != GameState::BIDDING) return events;

    biddingPlayers.insert(playerId);
    for (auto& p : players) if (p.id == playerId) p.wantsSam = wantSam;

    // Broadcast cho làng biết
    json jUpdate; jUpdate["userId"] = playerId; jUpdate["wantsSam"] = wantSam;
    GameEvent evUpd; evUpd.type = MessageType::S_BAO_SAM_RESULT; // Tạm dùng msg type này
    evUpd.payload = jUpdate.dump(); evUpd.targetPlayerIds = getAllPlayerIds(); evUpd.roomId = roomId; 
    events.push_back(evUpd);

    // Luật: Báo phát là chốt luôn người đi đầu (First Come First Serve)
    if (wantSam) {
        Logger::log(LogLevel::INFO, "SAMLOC", "Player " + std::to_string(playerId) + " Bao Sam SUCCESS!");
        auto endEvs = endBiddingPhase(playerId);
        events.insert(events.end(), endEvs.begin(), endEvs.end());
        return events;
    }
    if (biddingPlayers.size() == players.size()) {
        auto endEvs = endBiddingPhase(-1);
        events.insert(events.end(), endEvs.begin(), endEvs.end());
        return events;
    }
    return events;
}

std::vector<GameEvent> SamLocGame::endBiddingPhase(int samWinnerId) {
    std::vector<GameEvent> events;
    int startId = -1;
    
    if (samWinnerId != -1) {
        startId = samWinnerId;
        // Gửi thông báo CHÍNH THỨC ai là người Báo Sâm được đi đầu
        json jRes; jRes["userId"] = samWinnerId; 
        GameEvent ev; ev.type = MessageType::S_BAO_SAM_RESULT; ev.payload = jRes.dump();
        ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; 
        events.push_back(ev);
    } else {
        // Logic tìm bài nhỏ nhất
        int minScore = 99999; startId = players[0].id;
        for (const auto& p : players) {
            for (const auto& c : p.hand) {
                int suitVal = (c.suit=="spade"?0 : (c.suit=="club"?1 : (c.suit=="diamond"?2 : 3)));
                int score = c.orderValue * 10 + suitVal;
                if (score < minScore) { minScore = score; startId = p.id; }
            }
        }
        json jRes; jRes["userId"] = -1; // Không ai báo
        GameEvent ev; ev.type = MessageType::S_BAO_SAM_RESULT; ev.payload = jRes.dump();
        ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; 
        events.push_back(ev);
    }
    
    auto playEvs = startPlayingPhase(startId);
    events.insert(events.end(), playEvs.begin(), playEvs.end());
    return events;
}

std::vector<GameEvent> SamLocGame::startPlayingPhase(int firstPlayerId) {
    std::vector<GameEvent> events;
    state = GameState::PLAYING;
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].id == firstPlayerId) { currentPlayerIndex = (int)i; break; }
    }
    lastTurnOwnerId = -1;

    json jTurn; jTurn["currentPlayerId"] = firstPlayerId; jTurn["timeout"] = GAME_TURN_TIMEOUT_SECONDS;
    GameEvent ev; ev.type = MessageType::S_TURN_INFO; ev.payload = jTurn.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);

    phaseStartTime = std::chrono::steady_clock::now();
    return events;
}

// === [LOGIC QUAN TRỌNG] FAST HANDS / CHẶN SÂM ===
std::vector<GameEvent> SamLocGame::playCards(int playerId, std::vector<int> cardIds) {
    std::vector<GameEvent> events; 
    if (state != GameState::PLAYING) return events;
    
    // Check xem có người đang Báo Sâm không
    int baoSamId = -1;
    for(auto& p : players) if(p.wantsSam) baoSamId = p.id;

    // --- LOGIC THƯỜNG (Không ai báo sâm) ---
    if (baoSamId == -1) {
        if (players.empty() || players[currentPlayerIndex].id != playerId) return events;
        // ... (Logic đánh bài thường như cũ) ...
    }
    // --- LOGIC BÁO SÂM (Chế độ Fast Hands) ---
    else {
        // Nếu là người Báo Sâm đánh: Phải đúng lượt (Mà thực ra lúc nào chả là lượt nó nếu chưa bị chặn)
        if (playerId == baoSamId) {
            if (players[currentPlayerIndex].id != playerId) return events; // Safety
        } 
        // Nếu là NGƯỜI KHÁC đánh (Chặn Sâm)
        else {
            // Chỉ được chặn khi lượt vừa rồi là của thằng Sâm (lastTurnOwnerId == baoSamId)
            // Và thằng Sâm chưa đánh hết bài
            if (lastTurnOwnerId != baoSamId) {
                // Chưa đến lúc chặn (hoặc thằng Sâm chưa đánh bài nào để mà chặn)
                return events; 
            }
            // Không check currentPlayerIndex ở đây -> "Thằng nào nhanh tay thì được"
            Logger::log(LogLevel::INFO, "SAMLOC", "Player " + std::to_string(playerId) + " trying to BLOCK SAM of " + std::to_string(baoSamId));
        }
    }

    // --- Validation & Execute ---
    Player* pSender = nullptr;
    for(auto& p : players) if(p.id == playerId) pSender = &p;
    if(!pSender) return events;

    std::vector<DBCard> cardsToPlay;
    for (int cid : cardIds) {
        auto it = std::find_if(pSender->hand.begin(), pSender->hand.end(), [cid](const DBCard& c){ return c.id == cid; });
        if (it != pSender->hand.end()) cardsToPlay.push_back(*it); else return events;
    }
    std::string err; if (!isValidMove(cardsToPlay, err)) return events;

    // TODO: Ở đây bạn cần logic check beat (bài đánh ra có chặt được bài cũ không)
    // Nếu là chặn Sâm mà bài không chặn được -> Ignore (coi như đánh sai)
    
    // Execute Move
    try { gameRepo.logMove(currentGameId, playerId, "play", vecToString(cardIds)); } catch (...) {}
    
    for (int cid : cardIds) {
        auto itRem = std::remove_if(pSender->hand.begin(), pSender->hand.end(), [cid](const DBCard& c){ return c.id == cid; });
        pSender->hand.erase(itRem, pSender->hand.end());
    }
    lastTurnOwnerId = playerId; // Cập nhật người vừa đánh

    // Broadcast Move
    json jMove; jMove["userId"] = playerId; jMove["cards"] = cardIds; jMove["cardsLeft"] = pSender->hand.size();
    GameEvent ev; ev.type = MessageType::S_MOVE_RESULT; ev.payload = jMove.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);

    // --- XỬ LÝ KẾT QUẢ SÂM ---
    if (baoSamId != -1) {
        // 1. Nếu người đánh là người chặn Sâm (không phải ông Sâm)
        if (playerId != baoSamId) {
            Logger::log(LogLevel::INFO, "SAMLOC", "SAM BLOCKED! Player " + std::to_string(playerId) + " blocked " + std::to_string(baoSamId));
            
            // Ông Sâm thua Đền Sâm (Reason 4), nhưng winner là ông Chặn (playerId)
            // Ta sẽ gọi hàm handleWin với winnerId = playerId (người chặn), và reason = 5 (Block Sam Success)
            // Để logic tính tiền biết mà chia
            auto winEvs = handleWin(playerId, 5); 
            events.insert(events.end(), winEvs.begin(), winEvs.end());
            return events;
        }
        
        // 2. Nếu người đánh là ông Sâm và hết bài -> Thắng Sâm
        if (pSender->hand.empty()) {
            Logger::log(LogLevel::INFO, "SAMLOC", "SAM SUCCESS! Player " + std::to_string(playerId) + " finished hand.");
            auto winEvs = handleWin(playerId, 3); // 3 = Tới trắng / Sâm thành công
            events.insert(events.end(), winEvs.begin(), winEvs.end());
            return events;
        }

        // 3. Ông Sâm đánh xong mà chưa hết bài -> Vẫn là lượt ông Sâm (vì Sâm đi tiếp)
        // Reset time cho ông Sâm đánh tiếp, chờ người khác chặn
        // Ở chế độ Sâm, currentPlayerIndex luôn giữ ở ông Sâm cho đến khi bị chặn hoặc hết bài
        // Tuy nhiên, để client hiển thị đúng, ta vẫn gửi S_TURN_INFO trỏ về ông Sâm
        json jTurn; jTurn["currentPlayerId"] = baoSamId; jTurn["timeout"] = GAME_TURN_TIMEOUT_SECONDS;
        GameEvent evT; evT.type = MessageType::S_TURN_INFO; evT.payload = jTurn.dump();
        evT.targetPlayerIds = getAllPlayerIds(); evT.roomId = roomId; events.push_back(evT);
        phaseStartTime = std::chrono::steady_clock::now();
        return events;
    }

    // --- LOGIC THƯỜNG ---
    if (pSender->hand.empty()) {
        auto winEvs = handleWin(playerId, 1); events.insert(events.end(), winEvs.begin(), winEvs.end());
    } else {
        auto nextEvs = nextTurn(); events.insert(events.end(), nextEvs.begin(), nextEvs.end());
    }
    return events;
}

std::vector<GameEvent> SamLocGame::passTurn(int playerId) {
    std::vector<GameEvent> events; if (state != GameState::PLAYING) return events;
    
    // Nếu đang Báo Sâm -> Không được Pass (trừ khi đang đánh thường)
    // Thực tế trong Báo Sâm, người khác không Pass, chỉ có Chặn hoặc Im lặng.
    // Nếu họ gửi Pass -> Kệ, hoặc coi như bỏ qua cơ hội chặn lượt này.
    
    players[currentPlayerIndex].hasPassed = true;
    try { gameRepo.logMove(currentGameId, playerId, "pass", "[]"); } catch (...) {}

    json jPass; jPass["userId"] = playerId; jPass["action"] = "pass";
    GameEvent ev; ev.type = MessageType::S_MOVE_RESULT; ev.payload = jPass.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);

    auto nextEvs = nextTurn();
    events.insert(events.end(), nextEvs.begin(), nextEvs.end());
    return events;
}

std::vector<GameEvent> SamLocGame::nextTurn() {
    std::vector<GameEvent> events; if (players.empty()) return events;
    int next = currentPlayerIndex; int loop = 0;
    do {
        next = (next + 1) % (int)players.size(); ++loop;
    } while ((players[next].hasPassed || players[next].isDisconnected) && loop < (int)players.size());

    if (loop >= (int)players.size() || players[next].id == lastTurnOwnerId) {
        for (auto& p : players) if(!p.isDisconnected) p.hasPassed = false;
        lastTurnOwnerId = -1;
    }
    currentPlayerIndex = next;
    
    json jTurn; jTurn["currentPlayerId"] = players[next].id; jTurn["timeout"] = GAME_TURN_TIMEOUT_SECONDS;
    GameEvent ev; ev.type = MessageType::S_TURN_INFO; ev.payload = jTurn.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);
    
    phaseStartTime = std::chrono::steady_clock::now();
    return events;
}

// === [TÍNH TIỀN] ===
std::vector<GameEvent> SamLocGame::handleWin(int winnerId, int reasonCode) {
    // Reason 3: Sâm Thành Công (Winner ăn mỗi nhà 20 lần)
    // Reason 4: Đền Sâm do Disconnect (Winner = Loser, bị phạt tiền chia đều)
    // Reason 5: Đền Sâm do Bị Chặn (Winner = Người chặn, ăn TRỌN tiền phạt của thằng Sâm)
    
    std::vector<GameEvent> events;
    state = GameState::FINISHED;
    try { gameRepo.endGame(currentGameId, winnerId); } catch (...) {}

    json resultsJson = json::array();
    double baseBet = roomInfo.betAmount; 
    
    // --- CASE 5: SÂM BỊ CHẶN (Blocker ăn hết) ---
    if (reasonCode == 5) {
        int blockerId = winnerId; // Người chặn được
        int samLoserId = -1;
        for(auto& p : players) if(p.wantsSam) samLoserId = p.id; // Tìm thằng báo sâm

        double penaltyPerPerson = 20.0 * baseBet; 
        // Tổng phạt = 20 * Bet * (Số người - 1)
        int losersCount = countActivePlayers() - 1;
        double totalPenalty = penaltyPerPerson * losersCount;

        for (auto& p : players) {
            if (p.id == samLoserId) {
                // Thằng Sâm thua đậm
                try { playerRepo.updateBalance(p.id, -totalPenalty); gameRepo.saveResult(currentGameId, p.id, 2, -totalPenalty, p.hand.size()); } catch(...) {}
                json jL; jL["userId"] = p.id; jL["rank"] = 2; jL["change"] = -totalPenalty; jL["cardsLeft"] = p.hand.size();
                resultsJson.push_back(jL);
            } else if (p.id == blockerId) {
                // Thằng Chặn ăn hết
                try { playerRepo.updateBalance(p.id, totalPenalty); gameRepo.saveResult(currentGameId, p.id, 1, totalPenalty, p.hand.size()); } catch(...) {}
                json jW; jW["userId"] = p.id; jW["rank"] = 1; jW["change"] = totalPenalty; jW["cardsLeft"] = p.hand.size();
                resultsJson.push_back(jW);
            } else {
                // Mấy thằng khác hòa tiền (hoặc tính logic khác tùy, ở đây cho hòa theo ý bạn "thằng chặn liếm hết")
                json jN; jN["userId"] = p.id; jN["rank"] = 2; jN["change"] = 0; jN["cardsLeft"] = p.hand.size();
                resultsJson.push_back(jN);
            }
        }
    }
    // --- CASE 4: SÂM FAIL (Disconnect/Timeout) ---
    else if (reasonCode == 4) {
        int loserId = winnerId; // ID truyền vào là người bị phạt
        double totalLost = 0;
        double penaltyPerPerson = 20.0 * baseBet;

        for (auto& p : players) {
            if (p.id == loserId) continue;
            // Những người còn lại được chia tiền phạt
            double winAmt = penaltyPerPerson; 
            // Nếu là đếm lá, vẫn có thể cộng thêm bài thối của họ
            try { playerRepo.updateBalance(p.id, winAmt); gameRepo.saveResult(currentGameId, p.id, 1, winAmt, p.hand.size()); } catch(...) {}
            totalLost += winAmt;
            json jR; jR["userId"] = p.id; jR["rank"] = 1; jR["change"] = winAmt; jR["cardsLeft"] = p.hand.size();
            resultsJson.push_back(jR);
        }
        try { playerRepo.updateBalance(loserId, -totalLost); gameRepo.saveResult(currentGameId, loserId, 2, -totalLost, 0); } catch(...) {}
        json jL; jL["userId"] = loserId; jL["rank"] = 2; jL["change"] = -totalLost; jL["cardsLeft"] = 0;
        resultsJson.push_back(jL);
        winnerId = -1; 
    }
    // --- CASE 3: SÂM SUCCESS (Tới trắng) ---
    else if (reasonCode == 3) {
        double totalWin = 0;
        for (auto& p : players) {
            if (p.id == winnerId) continue;
            double lost = -20.0 * baseBet; // Thua 20 lần
            // Cộng thêm bài thối heo
            for(auto& c:p.hand) if(c.rank=="2") lost -= (5.0*baseBet);
            
            try { playerRepo.updateBalance(p.id, lost); gameRepo.saveResult(currentGameId, p.id, 2, lost, p.hand.size()); } catch(...) {}
            totalWin += std::abs(lost);
            json jL; jL["userId"] = p.id; jL["rank"] = 2; jL["change"] = lost; jL["cardsLeft"] = p.hand.size();
            resultsJson.push_back(jL);
        }
        try { playerRepo.updateBalance(winnerId, totalWin); gameRepo.saveResult(currentGameId, winnerId, 1, totalWin, 0); } catch(...) {}
        json jW; jW["userId"] = winnerId; jW["rank"] = 1; jW["change"] = totalWin; jW["cardsLeft"] = 0;
        resultsJson.push_back(jW);
    }
    // --- CASE 1: THẮNG THƯỜNG ---
    else {
        double totalWin = 0;
        for (const auto& p : players) {
            if (p.id == winnerId) continue;
            double lostAmount = 0; int cardsLeft = (int)p.hand.size();
            if (p.isDisconnected) lostAmount = -baseBet;
            else {
                if (roomInfo.type == "dem_la") {
                     double pen = (cardsLeft==10)?(15.0*baseBet):(cardsLeft*baseBet);
                     lostAmount = -pen;
                } else lostAmount = -baseBet;
                for(const auto& c:p.hand) if(c.rank=="2") lostAmount -= (5.0*baseBet);
            }
            try { playerRepo.updateBalance(p.id, lostAmount); gameRepo.saveResult(currentGameId, p.id, 2, lostAmount, cardsLeft); } catch(...) {}
            totalWin += std::abs(lostAmount);
            json jR; jR["userId"] = p.id; jR["rank"] = 2; jR["change"] = lostAmount; jR["cardsLeft"] = cardsLeft;
            resultsJson.push_back(jR);
        }
        try { playerRepo.updateBalance(winnerId, totalWin); gameRepo.saveResult(currentGameId, winnerId, 1, totalWin, 0); } catch(...) {}
        json jW; jW["userId"] = winnerId; jW["rank"] = 1; jW["change"] = totalWin; jW["cardsLeft"] = 0;
        resultsJson.push_back(jW);
    }

    json jFinal; jFinal["winnerId"] = winnerId; jFinal["results"] = resultsJson;
    GameEvent ev; ev.type = MessageType::S_GAME_END; ev.payload = jFinal.dump();
    ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);
    return events;
}

std::vector<GameEvent> SamLocGame::update() {
    std::vector<GameEvent> events;
    auto now = std::chrono::steady_clock::now();

    if (state == GameState::WAITING && startingScheduled) {
        if (now >= scheduledStartTime) {
            auto evs = startBiddingPhase();
            events.insert(events.end(), evs.begin(), evs.end());
            return events;
        }
    }
    if (state == GameState::BIDDING) {
        double elapsed = std::chrono::duration<double>(now - phaseStartTime).count();
        if (elapsed > GAME_BIDDING_TIMEOUT_SECONDS) {
            auto evs = endBiddingPhase(-1);
            events.insert(events.end(), evs.begin(), evs.end());
            return events;
        }
    }
    if (state == GameState::PLAYING) {
        double elapsed = std::chrono::duration<double>(now - phaseStartTime).count();
        if (elapsed > GAME_TURN_TIMEOUT_SECONDS) {
            if (!players.empty()) {
                int pid = players[currentPlayerIndex].id;
                // Nếu báo Sâm mà timeout -> Thua đền sâm
                if (players[currentPlayerIndex].wantsSam) {
                    auto loseEvs = handleWin(pid, 4);
                    events.insert(events.end(), loseEvs.begin(), loseEvs.end());
                    return events;
                }
                // Auto pass thường
                players[currentPlayerIndex].hasPassed = true;
                json jPass; jPass["userId"] = pid; jPass["action"] = "auto_pass";
                GameEvent ev; ev.type = MessageType::S_MOVE_RESULT; ev.payload = jPass.dump();
                ev.targetPlayerIds = getAllPlayerIds(); ev.roomId = roomId; events.push_back(ev);
                auto nextEvs = nextTurn();
                events.insert(events.end(), nextEvs.begin(), nextEvs.end());
                return events;
            }
        }
    }
    return events;
}