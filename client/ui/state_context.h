#pragma once
#include <nlohmann/json.hpp>
#include "game_state.h"
#include "../core/network_client.h"
#include "../core/thread_safe_queue.h"
#include "../core/network_event.h"
#include "../handlers/session/client_session.h"
#include "../handlers/chat/chat_handler.h"
#include "../handlers/auth/auth_handler.h"
#include "../handlers/rank/rank_handler.h"
#include "../handlers/friend/friend_handler.h"
#include "../handlers/lucky_wheel/lucky_wheel_handler.h"
#include "../handlers/room/room_handler.h"
#include <memory>
#include <functional>
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>
#include "../handlers/room/room_structs.h"

class NetworkClient;
class ClientSession;
class ChatHandler;
class RankHandler;
class AuthHandler;
class RoomHandler;
template<typename T> class ThreadSafeQueue;
struct NetworkEvent;

enum class GameStateType {
    Login,
    Lobby,
    PrivateChat,
    Ranking,
    RoomList,
    WaitingRoom,
    GameStartingCountdown,
    InGame,
    Friends,
    LuckyWheel,
    GameResult
};

using StateTransitionCallback = std::function<void(GameStateType)>;

class StateContext {
public:
    NetworkClient& network;
    ClientSession& session;
    ChatHandler& chatHandler;
    RankHandler& rankHandler;
    LuckyWheelHandler& luckyWheelHandler;
    ThreadSafeQueue<NetworkEvent>& eventQueue;
    StateTransitionCallback requestTransition;
    AuthHandler& auth_handler;
    RoomHandler& roomHandler;
    RoomInfo currentRoomInfo;
    int currentRoomId = 0;
    std::vector<int> myHand; 
    
    std::vector<RoomMember> currentRoomMembers;
    nlohmann::json lastGameResult;
    int currentPlayerTurnId = 0; 
    int currentTurnTimeout = 0;
    FriendHandler& friendHandler;
    sf::Font& font;

    StateContext(NetworkClient& net,
                 ClientSession& sess,
                 ChatHandler& chat_h,
                 RankHandler& rank_h,
                 LuckyWheelHandler& lucky_wheel_h,
                 ThreadSafeQueue<NetworkEvent>& eq,
                 AuthHandler& auth_h,
                 FriendHandler& friend_h,
                 RoomHandler& rHandler, sf::Font& f)
        : network(net),
          session(sess),
          chatHandler(chat_h),
          rankHandler(rank_h),
          luckyWheelHandler(lucky_wheel_h),
          eventQueue(eq),
          requestTransition(nullptr),
          auth_handler(auth_h),
          friendHandler(friend_h),
          roomHandler(rHandler), 
          font(f) {}
};

#include "../core/network_client.h"
#include "../core/thread_safe_queue.h"
#include "../core/network_event.h"
#include "../handlers/session/client_session.h"
#include "../handlers/chat/chat_handler.h"
#include "../handlers/auth/auth_handler.h"
#include "../handlers/rank/rank_handler.h"
#include "../handlers/room/room_handler.h"
#include "../handlers/room/room_structs.h"