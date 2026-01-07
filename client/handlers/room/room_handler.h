#pragma once
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>
#include "../../core/network_event.h"
#include "room_structs.h"

using json = nlohmann::json;

using RoomListCallback = std::function<void(const std::vector<RoomInfo>&)>;
using RoomMembersCallback = std::function<void(const std::vector<RoomMember>&)>;
using JoinRoomCallback = std::function<void(bool, const std::string&, int, const RoomInfo&)>; 
using GameCountdownCallback = std::function<void(int)>;
using GameStartCallback = std::function<void(const std::vector<int>&)>;
using TurnInfoCallback = std::function<void(int, int)>;
using MoveResultCallback = std::function<void(int, const std::vector<int>&, int, const std::string&)>;
using GameResultCallback = std::function<void(const nlohmann::json&)>;

class RoomHandler {
public:
    void setRoomListCallback(RoomListCallback cb);
    void setRoomUpdateCallback(RoomMembersCallback cb);
    void setJoinRoomCallback(JoinRoomCallback cb);
    void setGameCountdownCallback(GameCountdownCallback cb);
    void setGameStartCallback(GameStartCallback cb);
    void setTurnInfoCallback(TurnInfoCallback cb);
    void setMoveResultCallback(MoveResultCallback cb);
    void setGameResultCallback(GameResultCallback cb); 

    void onRoomListReceived(const Message& msg);
    void onRoomUpdateReceived(const Message& msg);
    void onJoinRoomResult(const Message& msg, bool isSuccess);
    void onGameCountdownReceived(const Message& msg);
    void onGameStartReceived(const Message& msg);
    void onMoveResultReceived(const Message& msg);
    void onGameEndReceived(const Message& msg); 

private:
    std::vector<RoomInfo> parseRooms(const std::string& payload);
    std::vector<RoomMember> parseMembers(const std::string& payload);

    RoomListCallback listCallback_;
    RoomMembersCallback updateCallback_;
    JoinRoomCallback joinCallback_;
    GameCountdownCallback countdownCallback_;
    GameStartCallback gameStartCallback_;
    TurnInfoCallback turnInfoCallback_;
    MoveResultCallback moveResultCallback_;
    GameResultCallback gameResultCallback_;
};