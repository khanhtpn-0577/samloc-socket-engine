#include "game_handler.h"
#include "../connection/connection_handler.h"
#include "../session/session_manager.h"
#include "../../utils/logger.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

std::unordered_map<int, int> GameHandler::userRoomMap;

std::string GameHandler::intToHex(uint16_t n) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << n;
    return ss.str();
}

void GameHandler::init(Database& db) {
    static bool initialized = false;
    if (initialized) return; 
    
    Logger::log(LogLevel::INFO, "INIT", "Initializing GameHandler...");
    try {
        roomRepo = new RoomRepository(db);
        playerRepo = new PlayerRepository(db);
        cardRepo = new CardRepository(db);
        gameRepo = new GameRepository(db);
        globalDeck = cardRepo->getAllCards();
        initialized = true;
        Logger::log(LogLevel::INFO, "INIT", "GameHandler initialized. Deck size: " + std::to_string(globalDeck.size()));
    } catch (const std::exception& e) {
        Logger::log(LogLevel::ERROR, "INIT", "Initialization failed: " + std::string(e.what()));
    }
}

void GameHandler::clientDisconnected(int userId) {
    onClientDisconnect(userId);
}

Message GameHandler::handleMessage(const Message& msg, int senderId) {
    MessageType type = static_cast<MessageType>(msg.header.messageType);
    std::string payload = msg.payload;
    std::vector<GameEvent> events;

    Logger::log(LogLevel::DEBUG, "HANDLER", "Received message | User: " + std::to_string(senderId) + " | Type: 0x" + intToHex((uint16_t)type));

    json jPayload;
    if (!payload.empty()) {
        try { jPayload = json::parse(payload); } catch (...) { if (type != MessageType::C_GET_ROOM_LIST) return Message(); }
    }

    if (type == MessageType::C_GET_ROOM_LIST) {
        std::vector<DBRoom> allRooms = roomRepo->getAllRooms();
        json jRooms = json::array();
        for (auto& r : allRooms) {
            int curr = 0;
            std::string status = "waiting";
            if (games.find(r.id) != games.end()) {
                curr = games[r.id]->getCurrentPlayerCount();
                if (!games[r.id]->isJoinAllowed()) status = "playing";
            }
            jRooms.push_back({
                {"id", r.id}, {"name", r.roomName}, {"type", r.type},
                {"bet", (long long)r.betAmount}, {"max", r.maxPlayers},
                {"curr", curr}, {"status", status}
            });
        }
        Message resp;
        resp.header.messageType = (uint16_t)MessageType::S_ROOM_LIST;
        resp.payload = json({{"rooms", jRooms}}).dump();
        resp.header.payloadLength = resp.payload.size();
        return resp;
    }

    if (type == MessageType::C_JOIN_ROOM) {
        int roomId = jPayload.value("roomId", 0);
        if (userRoomMap.count(senderId) && userRoomMap[senderId] != roomId) {
            int oldRid = userRoomMap[senderId];
            if (games.count(oldRid) && games[oldRid]->getState() == GameState::WAITING) {
                dispatchEvents(games[oldRid]->removePlayer(senderId));
                userRoomMap.erase(senderId);
            }
        }

        DBRoom roomInfo;
        if (!roomRepo->getRoom(roomId, roomInfo)) return Message();

        if (games.find(roomId) == games.end()) {
            games[roomId] = std::make_shared<SamLocGame>(roomId, roomInfo, globalDeck, *playerRepo, *gameRepo);
        }

        auto game = games[roomId];
        if (!game->isPlayerInGame(senderId)) {
            if (game->getCurrentPlayerCount() >= roomInfo.maxPlayers || !game->isJoinAllowed()) return Message();
            events = game->addPlayer(senderId);
        }

        userRoomMap[senderId] = roomId;
        try { roomRepo->addPlayerToRoom(roomId, senderId); } catch(...) {}

        Message msgEx;
        msgEx.header.messageType = (uint16_t)MessageType::S_EXISTING_PLAYERS;
        msgEx.payload = game->getPlayersStateJsonFor(senderId);
        if (auto* c = SessionManager::instance().get(senderId)) c->sendMessage(msgEx);

        if (!events.empty()) {
            dispatchEvents(events);
        }

        return Message();
    }

    if (userRoomMap.count(senderId)) {
        int rId = userRoomMap[senderId];
        if (games.count(rId)) {
            auto game = games[rId];
            switch (type) {
                case MessageType::C_READY:
                    events = game->setPlayerReady(senderId, jPayload.value("ready", jPayload.value("isReady", false)));
                    break;
                case MessageType::C_PLAY_CARD:
                    events = game->playCards(senderId, jPayload.value("cards", std::vector<int>{}));
                    break;
                case MessageType::C_PASS_TURN:
                    events = game->passTurn(senderId);
                    break;
                case MessageType::C_LEAVE_ROOM:
                    events = game->removePlayer(senderId); 
                    userRoomMap.erase(senderId);
                    try { roomRepo->removePlayerFromRoom(rId, senderId); } catch(...) {}
                    if (game->getCurrentPlayerCount() == 0) games.erase(rId);
                    break;
                default: break;
            }
        }
    }
    if (!events.empty()) dispatchEvents(events);
    return Message();
}

void GameHandler::dispatchEvents(const std::vector<GameEvent>& events) {
    for (const auto& ev : events) {
        Message resp;
        resp.header.messageType = static_cast<uint16_t>(ev.type);
        resp.payload = ev.payload;
        resp.header.payloadLength = resp.payload.size();

        std::vector<int> targets = ev.targetPlayerIds;
        if (targets.empty() && games.count(ev.roomId)) {
            targets = games[ev.roomId]->getAllPlayerIds();
        }

        for (int uid : targets) {
            if (auto* conn = SessionManager::instance().get(uid)) {
                conn->sendMessage(resp);
                std::string typeHex = intToHex(resp.header.messageType);
                Logger::log(LogLevel::DEBUG, "DISPATCH", 
                    "[SEND] -> User: " + std::to_string(uid) + 
                    " | Type: 0x" + typeHex + 
                    " | Payload: " + resp.payload);
            }
        }
    }
}

std::vector<GameEvent> GameHandler::onClientDisconnect(int userId) {
    std::vector<GameEvent> allEvents;
    if (userRoomMap.find(userId) == userRoomMap.end()) return allEvents;

    int rId = userRoomMap[userId];
    userRoomMap.erase(userId);

    if (games.count(rId)) {
        auto game = games[rId];
        allEvents = game->onPlayerDisconnect(userId);
        
        GameEvent syncEv;
        syncEv.type = MessageType::S_ROOM_UPDATE;
        syncEv.payload = game->getPlayersStateJsonFor(-1);
        syncEv.roomId = rId;
        syncEv.targetPlayerIds = game->getAllPlayerIds();
        allEvents.push_back(syncEv);

        try { roomRepo->removePlayerFromRoom(rId, userId); } catch(...) {}
        if (game->getCurrentPlayerCount() == 0) games.erase(rId);
    }

    if (!allEvents.empty()) dispatchEvents(allEvents);
    return allEvents;
}

void GameHandler::updateLoop() {
    for (auto it = games.begin(); it != games.end();) {
        auto game = it->second;
        if (game) {
            auto evs = game->update();
            if (!evs.empty()) dispatchEvents(evs);
            ++it;
        } else it = games.erase(it);
    }
}

