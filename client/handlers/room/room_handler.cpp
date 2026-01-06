#include "room_handler.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

void RoomHandler::setRoomListCallback(RoomListCallback cb) { listCallback_ = cb; }
void RoomHandler::setRoomUpdateCallback(RoomMembersCallback cb) { updateCallback_ = cb; }
void RoomHandler::setJoinRoomCallback(JoinRoomCallback cb) { joinCallback_ = cb; }
void RoomHandler::setGameCountdownCallback(GameCountdownCallback cb) { countdownCallback_ = cb; }
void RoomHandler::setGameStartCallback(GameStartCallback cb) { gameStartCallback_ = cb; }
void RoomHandler::setTurnInfoCallback(TurnInfoCallback cb) { turnInfoCallback_ = cb; }
void RoomHandler::setMoveResultCallback(MoveResultCallback cb) { moveResultCallback_ = cb; }

void RoomHandler::onRoomListReceived(const Message& msg) {
    if (listCallback_) listCallback_(parseRooms(msg.payload));
}

void RoomHandler::onRoomUpdateReceived(const Message& msg) {
    if (msg.payload.empty() || msg.payload == "{}") return;
    if (updateCallback_) updateCallback_(parseMembers(msg.payload));
}

void RoomHandler::onJoinRoomResult(const Message& msg, bool isSuccess) {
    std::string message = "";
    int roomId = 0;
    if (!isSuccess) {
        try {
            auto j = json::parse(msg.payload);
            message = j.value("reason", j.value("msg", "Error"));
        } catch (...) { message = "Error"; }
    }
    if (joinCallback_) joinCallback_(isSuccess, message, roomId, {});
}

void RoomHandler::onGameCountdownReceived(const Message& msg) {
    try {
        auto j = json::parse(msg.payload);
        if (countdownCallback_) countdownCallback_(j.value("startIn", 0));
    } catch (...) {}
}

void RoomHandler::onGameStartReceived(const Message& msg) {
    try {
        auto j = json::parse(msg.payload);
        if (gameStartCallback_) gameStartCallback_(j.value("hand", std::vector<int>{}));
    } catch (...) {}
}

void RoomHandler::onMoveResultReceived(const Message& msg) {
    try {
        auto j = json::parse(msg.payload);
        std::vector<RoomMember> updatedMembers;
        
        json jArr = j.contains("members") ? j["members"] : (j.contains("players") ? j["players"] : json::array());

        if (jArr.is_array()) {
            for (const auto& item : jArr) {
                RoomMember m;
                m.id = item.value("id", 0);
                m.name = item.value("name", "Player");
                m.handSize = item.value("handSize", item.value("hand_size", 0));
                if (item.contains("hand") && item["hand"].is_array()) {
                    m.hand = item["hand"].get<std::vector<int>>();
                }
                updatedMembers.push_back(m);
            }
            if (updateCallback_) updateCallback_(updatedMembers);
        }

        if (j.contains("nextTurnId") && turnInfoCallback_) {
            turnInfoCallback_(j.value("nextTurnId", 0), j.value("timeout", 20));
        }

        int userId = 0;
        std::vector<int> cardsPlayed;
        int cardsLeft = 0;
        std::string action = "play";

        if (j.contains("lastMove")) {
            const auto& lm = j["lastMove"];
            userId = lm.value("userId", 0);
            action = lm.value("action", "play");
            cardsPlayed = lm.value("cards", std::vector<int>{});
            for(const auto& m : updatedMembers) {
                if(m.id == userId) { cardsLeft = m.handSize; break; }
            }
        }

        if (userId != 0 && moveResultCallback_) {
            moveResultCallback_(userId, cardsPlayed, cardsLeft, action);
        }
    } catch (...) {}
}

std::vector<RoomInfo> RoomHandler::parseRooms(const std::string& payload) {
    std::vector<RoomInfo> rooms;
    try {
        auto j = json::parse(payload);
        if (j.contains("rooms") && j["rooms"].is_array()) {
            for (const auto& item : j["rooms"]) {
                RoomInfo r;
                r.id = item.value("id", 0);
                r.name = item.value("name", "");
                r.type = item.value("type", "");
                r.bet = item.value("bet", 0);
                r.currentPlayers = item.value("curr", 0);
                r.maxPlayers = item.value("max", 4);
                r.status = item.value("status", "waiting");
                rooms.push_back(r);
            }
        }
    } catch (...) {}
    return rooms;
}

std::vector<RoomMember> RoomHandler::parseMembers(const std::string& payload) {
    std::vector<RoomMember> members;
    try {
        auto jRoot = json::parse(payload);
        json jArr = jRoot.is_array() ? jRoot : (jRoot.contains("members") ? jRoot["members"] : (jRoot.contains("players") ? jRoot["players"] : json::array()));

        for (const auto& item : jArr) {
            RoomMember m;
            m.id = item.value("id", 0);
            m.name = item.value("name", "Player");
            m.balance = item.value("balance", 0LL);
            m.handSize = item.value("handSize", item.value("hand_size", 0));
            m.isReady = item.value("isReady", false);
            members.push_back(m);
        }
    } catch (...) {}
    return members;
}