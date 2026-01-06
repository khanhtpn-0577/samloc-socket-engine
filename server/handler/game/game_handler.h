#pragma once
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "../../net/protocol.h"
#include "../../db/database.h"
#include "../../db/repository/room_repository.h"
#include "../../db/repository/player_repository.h"
#include "../../db/repository/card_repository.h"
#include "../../db/repository/game_repository.h"
#include "../../logic/game/sam_loc_game.h"  // SamLocGame + DBCard
#include "../session/session_manager.h"

#include <unordered_map>
#include <memory>

class GameHandler {
public:
    static GameHandler& instance() {
        static GameHandler inst;
        return inst;
    }

    void init(Database& db);
    Message handleMessage(const Message& msg, int senderId);
    void updateLoop();
    void clientDisconnected(int userId); // public wrapper cho onClientDisconnect
    std::vector<GameEvent> onClientDisconnect(int userId);
private:
    GameHandler() = default;

    RoomRepository* roomRepo = nullptr;
    PlayerRepository* playerRepo = nullptr;
    CardRepository* cardRepo = nullptr;
    GameRepository* gameRepo = nullptr;

    std::vector<DBCard> globalDeck;

    // === ADD THIS ===
    std::unordered_map<int, std::shared_ptr<SamLocGame>> games; // roomId -> game instance

    static std::unordered_map<int,int> userRoomMap; // userId -> roomId

    void dispatchEvents(const std::vector<GameEvent>& events);
    int parseIntFromJson(const std::string& json, const std::string& key);
    bool parseBoolFromJson(const std::string& json, const std::string& key);
    std::vector<int> parseCardsFromJson(const std::string& json);
    static std::string intToHex(uint16_t n);
};
