#include "game_handler.h"
#include "../connection/connection_handler.h"
#include "../session/session_manager.h"
#include "../../utils/logger.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iostream>

using json = nlohmann::json;

// Map quản lý người chơi: UserId -> RoomId
std::unordered_map<int, int> GameHandler::userRoomMap;

void GameHandler::init(Database& db) {
    Logger::log(LogLevel::INFO, "INIT", "Initializing GameHandler components...");
    try {
        roomRepo = new RoomRepository(db);
        playerRepo = new PlayerRepository(db);
        cardRepo = new CardRepository(db);
        gameRepo = new GameRepository(db);
        globalDeck = cardRepo->getAllCards();
        Logger::log(LogLevel::INFO, "INIT", "GameHandler Init SUCCESS. Global Deck size: " + std::to_string(globalDeck.size()));
    } catch (const std::exception& e) {
        Logger::log(LogLevel::ERROR, "INIT", "GameHandler Init FAILED: " + std::string(e.what()));
    }
}

void GameHandler::clientDisconnected(int userId) { 
    Logger::log(LogLevel::INFO, "CONN", "ClientDisconnected callback triggered for User " + std::to_string(userId));
    onClientDisconnect(userId); 
}

Message GameHandler::handleMessage(const Message& msg, int senderId) {
    MessageType type = static_cast<MessageType>(msg.header.messageType);
    std::string payload = msg.payload;
    std::vector<GameEvent> events;

    // [LOG ENTRY]
    Logger::log(LogLevel::DEBUG, "HANDLER", "================================================================");
    Logger::log(LogLevel::DEBUG, "HANDLER", "RECV MSG | User: " + std::to_string(senderId) + " | Type: " + std::to_string((int)type) + " (0x" + std::to_string((int)type) + ")");
    Logger::log(LogLevel::DEBUG, "HANDLER", "PAYLOAD: " + (payload.empty() ? "[Empty]" : payload));

    json jPayload;
    if (!payload.empty()) {
        try { 
            jPayload = json::parse(payload); 
        } catch (const std::exception& e) { 
            Logger::log(LogLevel::ERROR, "JSON", "Parse Error: " + std::string(e.what()));
            if (type != MessageType::C_GET_ROOM_LIST) {
                Logger::log(LogLevel::WARNING, "HANDLER", "Aborting processing due to JSON error.");
                return Message(); 
            }
        }
    }

    /**
     * [DOCS] C_GET_ROOM_LIST
     * Input: {}
     * Output: S_ROOM_LIST {"rooms": [...]}
     */
    if (type == MessageType::C_GET_ROOM_LIST) {
        Logger::log(LogLevel::INFO, "ROOM_LIST", "User " + std::to_string(senderId) + " requested room list.");
        
        std::vector<DBRoom> allRooms = roomRepo->getAllRooms();
        Logger::log(LogLevel::DEBUG, "ROOM_LIST", "Found " + std::to_string(allRooms.size()) + " rooms in DB.");

        json jRooms = json::array();
        for (auto& r : allRooms) {
            int currentPlayers = 0; 
            std::string status = "waiting";
            
            if (games.find(r.id) != games.end()) {
                currentPlayers = games[r.id]->getCurrentPlayerCount();
                if (!games[r.id]->isJoinAllowed()) status = "playing";
            }
            
            json roomObj;
            roomObj["id"] = r.id; 
            roomObj["name"] = r.roomName; 
            roomObj["type"] = r.type;
            roomObj["bet"] = (long long)r.betAmount; 
            roomObj["max"] = r.maxPlayers;
            roomObj["curr"] = currentPlayers; 
            roomObj["status"] = status;
            jRooms.push_back(roomObj);
        }

        json jResp; jResp["rooms"] = jRooms;
        Message resp; 
        resp.header.messageType = (uint16_t)MessageType::S_ROOM_LIST;
        resp.payload = jResp.dump(); 
        resp.header.payloadLength = resp.payload.size();
        
        Logger::log(LogLevel::INFO, "ROOM_LIST", "Sending list to User " + std::to_string(senderId) + ". Payload size: " + std::to_string(resp.header.payloadLength));
        return resp;
    }

    int roomId = jPayload.value("roomId", 0);

    /**
     * [DOCS] C_JOIN_ROOM
     * Input: { "roomId": <int> }
     */
    if (type == MessageType::C_JOIN_ROOM) {
        Logger::log(LogLevel::INFO, "JOIN", "User " + std::to_string(senderId) + " requesting to JOIN Room " + std::to_string(roomId));

        // --- BYPASS LOGIC START ---
        if (userRoomMap.find(senderId) != userRoomMap.end()) {
            int oldRid = userRoomMap[senderId];
            Logger::log(LogLevel::WARNING, "JOIN_BYPASS", "User " + std::to_string(senderId) + " is already in Room " + std::to_string(oldRid));

            if (oldRid != roomId) {
                if (games.find(oldRid) != games.end()) {
                    auto oldGame = games[oldRid];
                    if (oldGame->getState() == GameState::WAITING) {
                        Logger::log(LogLevel::INFO, "JOIN_BYPASS", "Old Room " + std::to_string(oldRid) + " is WAITING. Executing Auto-Leave...");
                        
                        // Logic Remove
                        auto leaveEvents = oldGame->removePlayer(senderId);
                        Logger::log(LogLevel::DEBUG, "JOIN_BYPASS", "Generated " + std::to_string(leaveEvents.size()) + " leave events for Room " + std::to_string(oldRid));
                        dispatchEvents(leaveEvents); 
                        
                        try { roomRepo->removePlayerFromRoom(oldRid, senderId); } catch(...) {}
                        userRoomMap.erase(senderId);
                        Logger::log(LogLevel::INFO, "JOIN_BYPASS", "Removed User " + std::to_string(senderId) + " from DB/Map of Room " + std::to_string(oldRid));

                        if (oldGame->getCurrentPlayerCount() == 0) {
                            Logger::log(LogLevel::INFO, "JOIN_BYPASS", "Old Room " + std::to_string(oldRid) + " is empty. Deleting game instance.");
                            games.erase(oldRid);
                        }
                    } else {
                        Logger::log(LogLevel::WARNING, "JOIN_BYPASS", "Old Room " + std::to_string(oldRid) + " is PLAYING. Deny Join.");
                        Message err; err.header.messageType = (uint16_t)MessageType::ERROR_MESSAGE;
                        err.payload = "{\"code\":423,\"msg\":\"Playing elsewhere\"}";
                        err.header.payloadLength = err.payload.size(); return err;
                    }
                } else {
                    Logger::log(LogLevel::WARNING, "JOIN_BYPASS", "Old Room " + std::to_string(oldRid) + " game instance not found. Cleaning map.");
                    userRoomMap.erase(senderId);
                }
            } else {
                Logger::log(LogLevel::INFO, "JOIN", "User re-joining same room " + std::to_string(roomId) + " (Reconnect logic).");
            }
        }
        // --- BYPASS LOGIC END ---

        DBRoom roomInfo;
        if (!roomRepo->getRoom(roomId, roomInfo)) {
            Logger::log(LogLevel::WARNING, "JOIN", "Room ID " + std::to_string(roomId) + " not found in DB.");
            Message err; err.header.messageType = (uint16_t)MessageType::ERROR_MESSAGE;
            err.payload = "{\"code\":404,\"msg\":\"Room not found\"}";
            err.header.payloadLength = err.payload.size(); return err;
        }

        if (games.find(roomId) == games.end()) {
            Logger::log(LogLevel::INFO, "JOIN", "Creating new SamLocGame instance for Room " + std::to_string(roomId));
            games[roomId] = std::make_shared<SamLocGame>(roomId, roomInfo, globalDeck, *playerRepo, *gameRepo);
        }
        
        auto game = games[roomId];
        bool isReconnect = game->isPlayerInGame(senderId);
        Logger::log(LogLevel::DEBUG, "JOIN", "Reconnect check: " + std::string(isReconnect ? "TRUE" : "FALSE"));
        
        if (!isReconnect) {
            if (game->getCurrentPlayerCount() >= roomInfo.maxPlayers) {
                 Logger::log(LogLevel::WARNING, "JOIN", "Room " + std::to_string(roomId) + " is FULL.");
                 Message err; err.header.messageType = (uint16_t)MessageType::ERROR_MESSAGE;
                 err.payload = "{\"code\":423,\"msg\":\"Room Full\"}"; err.header.payloadLength = err.payload.size(); return err;
            }
            if (!game->isJoinAllowed()) {
                 Logger::log(LogLevel::WARNING, "JOIN", "Room " + std::to_string(roomId) + " is PLAYING/LOCKED.");
                 Message err; err.header.messageType = (uint16_t)MessageType::ERROR_MESSAGE;
                 err.payload = "{\"code\":423,\"msg\":\"Game Playing\"}"; err.header.payloadLength = err.payload.size(); return err;
            }
        }

        Logger::log(LogLevel::INFO, "JOIN", "Adding User " + std::to_string(senderId) + " to DB Room " + std::to_string(roomId));
        roomRepo->addPlayerToRoom(roomId, senderId);
        userRoomMap[senderId] = roomId;

        if (!isReconnect) {
            Logger::log(LogLevel::INFO, "JOIN", "Calling game->addPlayer for User " + std::to_string(senderId));
            events = game->addPlayer(senderId, "Player " + std::to_string(senderId));
        } else {
            Logger::log(LogLevel::INFO, "JOIN", "User " + std::to_string(senderId) + " reconnected. Syncing state.");
        }

        // Send Existing Players
        std::string exPlayers = game->getPlayersStateJson();
        Logger::log(LogLevel::DEBUG, "JOIN", "Sending S_EXISTING_PLAYERS to " + std::to_string(senderId) + ": " + exPlayers);
        
        Message msgEx;
        msgEx.header.messageType = (uint16_t)MessageType::S_EXISTING_PLAYERS;
        msgEx.payload = exPlayers;
        msgEx.header.payloadLength = msgEx.payload.size();
        auto* conn = SessionManager::instance().get(senderId);
        if (conn) conn->sendMessage(msgEx);
    } 
    // ================== IN-GAME ACTIONS ==================
    else if (userRoomMap.find(senderId) != userRoomMap.end()) {
        int rId = userRoomMap[senderId];
        Logger::log(LogLevel::DEBUG, "ACTION", "User " + std::to_string(senderId) + " mapped to Room " + std::to_string(rId));

        if (games.find(rId) != games.end()) {
            auto game = games[rId];
            try {
                switch (type) {
                    case MessageType::C_READY: {
                        bool r = jPayload.value("isReady", false);
                        Logger::log(LogLevel::INFO, "ACTION", "C_READY: User " + std::to_string(senderId) + " set " + (r?"TRUE":"FALSE"));
                        events = game->setPlayerReady(senderId, r); 
                        break;
                    }
                    case MessageType::C_BAO_SAM: {
                        bool w = jPayload.value("wantSam", false);
                        Logger::log(LogLevel::INFO, "ACTION", "C_BAO_SAM: User " + std::to_string(senderId) + " wants " + (w?"YES":"NO"));
                        events = game->handleBaoSam(senderId, w); 
                        break;
                    }
                    case MessageType::C_PLAY_CARD: {
                        std::vector<int> cards;
                        if (jPayload.contains("cards")) cards = jPayload["cards"].get<std::vector<int>>();
                        
                        std::string cardStr = "["; 
                        for(int c : cards) cardStr += std::to_string(c) + ","; 
                        cardStr += "]";
                        Logger::log(LogLevel::INFO, "ACTION", "C_PLAY_CARD: User " + std::to_string(senderId) + " played " + cardStr);
                        
                        events = game->playCards(senderId, cards);
                        break;
                    }
                    case MessageType::C_PASS_TURN: 
                        Logger::log(LogLevel::INFO, "ACTION", "C_PASS_TURN: User " + std::to_string(senderId) + " passed.");
                        events = game->passTurn(senderId); 
                        break;
                    
                    case MessageType::LEAVE_ROOM: 
                        Logger::log(LogLevel::INFO, "ACTION", "C_LEAVE_ROOM: User " + std::to_string(senderId) + " leaving.");
                        events = onClientDisconnect(senderId); 
                        break;
                    
                    default: 
                        Logger::log(LogLevel::WARNING, "ACTION", "Unknown/Unhandled MessageType in Game context: " + std::to_string((int)type));
                        break;
                }
            } catch (const std::exception& e) {
                Logger::log(LogLevel::ERROR, "ACTION", "Logic Exception processing action for " + std::to_string(senderId) + ": " + e.what());
            }
        } else {
            Logger::log(LogLevel::ERROR, "ACTION", "Game instance not found for Room " + std::to_string(rId) + ". Removing stale map.");
            userRoomMap.erase(senderId);
        }
    } else {
        Logger::log(LogLevel::WARNING, "HANDLER", "User " + std::to_string(senderId) + " sent Game Action but is NOT in any room map.");
    }

    if (!events.empty()) {
        Logger::log(LogLevel::INFO, "HANDLER", "Dispatching " + std::to_string(events.size()) + " events from action.");
        dispatchEvents(events);
    }
    
    return Message();
}

std::vector<GameEvent> GameHandler::onClientDisconnect(int userId) {
    std::vector<GameEvent> allEvents;
    if (userId == 0) return allEvents;

    if (userRoomMap.find(userId) == userRoomMap.end()) {
        Logger::log(LogLevel::DEBUG, "DISCONNECT", "User " + std::to_string(userId) + " disconnected but not in any room.");
        return allEvents;
    }

    int rId = userRoomMap[userId];
    Logger::log(LogLevel::INFO, "DISCONNECT", "Processing disconnect for User " + std::to_string(userId) + " from Room " + std::to_string(rId));
    
    userRoomMap.erase(userId);

    if (games.find(rId) != games.end()) {
        auto game = games[rId];
        
        // Logic Game
        auto evs = game->onPlayerDisconnect(userId);
        if (!evs.empty()) {
            Logger::log(LogLevel::DEBUG, "DISCONNECT", "Game logic generated " + std::to_string(evs.size()) + " events.");
            for (auto &e : evs) if (e.roomId == 0) e.roomId = rId;
            allEvents.insert(allEvents.end(), evs.begin(), evs.end());
        }

        // DB Cleanup
        try { 
            roomRepo->removePlayerFromRoom(rId, userId); 
            Logger::log(LogLevel::DEBUG, "DISCONNECT", "Removed User " + std::to_string(userId) + " from DB Room " + std::to_string(rId));
        } catch (const std::exception& e) {
            Logger::log(LogLevel::ERROR, "DISCONNECT", "DB Remove Failed: " + std::string(e.what()));
        }

        // Empty Room Cleanup
        if (game->getCurrentPlayerCount() == 0) {
            Logger::log(LogLevel::INFO, "DISCONNECT", "Room " + std::to_string(rId) + " is now EMPTY. Destroying Game Instance.");
            games.erase(rId); 
        } else {
            Logger::log(LogLevel::DEBUG, "DISCONNECT", "Room " + std::to_string(rId) + " still has " + std::to_string(game->getCurrentPlayerCount()) + " active players.");
        }
    } else {
        Logger::log(LogLevel::WARNING, "DISCONNECT", "Game instance for Room " + std::to_string(rId) + " not found!");
    }

    if (!allEvents.empty()) {
        dispatchEvents(allEvents);
    }
    return allEvents;
}

void GameHandler::updateLoop() {
    // Loop này chạy rất nhanh nên hạn chế log trừ khi có sự kiện
    for (auto it = games.begin(); it != games.end(); ) {
        int rid = it->first;
        auto game = it->second;
        
        if (game) {
            auto evs = game->update();
            if (!evs.empty()) {
                Logger::log(LogLevel::DEBUG, "UPDATE_LOOP", "Room " + std::to_string(rid) + " generated " + std::to_string(evs.size()) + " timed events.");
                for (auto &e : evs) if (e.roomId == 0) e.roomId = rid;
                dispatchEvents(evs);
            }
            ++it;
        } else {
            Logger::log(LogLevel::WARNING, "UPDATE_LOOP", "Null game pointer found for Room " + std::to_string(rid) + ". Removing.");
            it = games.erase(it);
        }
    }
}

void GameHandler::dispatchEvents(const std::vector<GameEvent>& events) {
    for (const auto& ev : events) {
        Message resp;
        resp.header.messageType = static_cast<uint16_t>(ev.type);
        resp.payload = ev.payload;
        resp.header.payloadLength = resp.payload.size();

        // Logger::log(LogLevel::DEBUG, "DISPATCH", "Type: 0x" + std::to_string((int)ev.type) + " Payload: " + ev.payload);

        if (ev.targetPlayerIds.empty()) {
            if (games.find(ev.roomId) != games.end()) {
                auto recipients = games[ev.roomId]->getAllPlayerIds();
                // Logger::log(LogLevel::DEBUG, "DISPATCH", "Broadcasting to Room " + std::to_string(ev.roomId) + " (" + std::to_string(recipients.size()) + " users)");
                for (int uid : recipients) {
                    auto* conn = SessionManager::instance().get(uid);
                    if (conn) conn->sendMessage(resp);
                }
            }
        } else {
            // Logger::log(LogLevel::DEBUG, "DISPATCH", "Sending to " + std::to_string(ev.targetPlayerIds.size()) + " targets.");
            for (int uid : ev.targetPlayerIds) {
                auto* conn = SessionManager::instance().get(uid);
                if (conn) conn->sendMessage(resp);
                // else Logger::log(LogLevel::WARNING, "DISPATCH", "Target User " + std::to_string(uid) + " not connected.");
            }
        }
    }
}

// --- Helper Functions ---
int GameHandler::parseIntFromJson(const std::string& jsonStr, const std::string& key) { try { return json::parse(jsonStr).value(key, 0); } catch(...) { return 0; } }
bool GameHandler::parseBoolFromJson(const std::string& jsonStr, const std::string& key) { try { return json::parse(jsonStr).value(key, false); } catch(...) { return false; } }
std::vector<int> GameHandler::parseCardsFromJson(const std::string& jsonStr) { try { return json::parse(jsonStr)["cards"].get<std::vector<int>>(); } catch(...) { return {}; } }