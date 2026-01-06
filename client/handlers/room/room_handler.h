#pragma once
#include "../../net/protocol.h"
#include "room_structs.h"
#include <functional>
#include <vector>
#include <string>

using RoomMembersCallback = std::function<void(const std::vector<RoomMember>&)>;
using RoomListCallback = std::function<void(const std::vector<RoomInfo>&)>;
using JoinRoomCallback = std::function<void(bool success, const std::string& message, int roomId, const RoomInfo& roomInfo)>;
using GameCountdownCallback = std::function<void(int seconds)>;
using GameStartCallback = std::function<void(const std::vector<int>& hand)>;
using TurnInfoCallback = std::function<void(int currentPlayerId, int timeout)>;
using MoveResultCallback = std::function<void(int, const std::vector<int>&, int, const std::string&)>;

class RoomHandler {
public:
    void onRoomListReceived(const Message& msg);
    void setRoomListCallback(RoomListCallback cb);

    void onRoomUpdateReceived(const Message& msg);
    void setRoomUpdateCallback(RoomMembersCallback cb);

    void onJoinRoomResult(const Message& msg, bool isSuccess);
    void setJoinRoomCallback(JoinRoomCallback cb);

    void onGameCountdownReceived(const Message& msg);
    void setGameCountdownCallback(GameCountdownCallback cb);

    void onGameStartReceived(const Message& msg);
    void setGameStartCallback(GameStartCallback cb);

    void onMoveResultReceived(const Message& msg);
    void setTurnInfoCallback(TurnInfoCallback cb);
    void setMoveResultCallback(MoveResultCallback cb);

private:
    RoomListCallback listCallback_;
    RoomMembersCallback updateCallback_;
    JoinRoomCallback joinCallback_;
    GameCountdownCallback countdownCallback_;
    GameStartCallback gameStartCallback_;
    TurnInfoCallback turnInfoCallback_;
    MoveResultCallback moveResultCallback_;

    std::vector<RoomInfo> parseRooms(const std::string& json);
    std::vector<RoomMember> parseMembers(const std::string& json);
    void parseMoveResult(const std::string& payload, int& userId, std::vector<int>& cardsPlayed, int& cardsLeft, std::string& action);
};