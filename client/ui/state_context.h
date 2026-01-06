#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>
#include "../handlers/room/room_structs.h"
// Forward declarations
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
    InGame
};

using StateTransitionCallback = std::function<void(GameStateType)>;

class StateContext {
public:
    NetworkClient& network;
    ClientSession& session;
    ChatHandler& chatHandler;
    RankHandler& rankHandler;
    ThreadSafeQueue<NetworkEvent>& eventQueue;
    StateTransitionCallback requestTransition;
    AuthHandler& auth_handler;
    RoomHandler& roomHandler;
    sf::Font& font;

    // Game-specific data shared between states
    RoomInfo currentRoomInfo;
    int currentRoomId = 0;
    std::vector<int> myHand; 
    
    // [NEW] Thông tin trạng thái bàn chơi (để InGame biết vẽ các người chơi khác)
    std::vector<RoomMember> currentRoomMembers;
    int currentPlayerTurnId = 0; // ID người đang đến lượt
    int currentTurnTimeout = 0; // Thời gian còn lại của lượt

    StateContext(NetworkClient& net, ClientSession& sess, ChatHandler& chat_handler, 
                 RankHandler& rank_handler, ThreadSafeQueue<NetworkEvent>& eq, 
                 AuthHandler& auth_handler, RoomHandler& rHandler, sf::Font& f)
        : network(net), 
          session(sess), 
          chatHandler(chat_handler), 
          rankHandler(rank_handler), 
          eventQueue(eq), 
          auth_handler(auth_handler), 
          roomHandler(rHandler), 
          font(f)
    {}
};

// Full includes after forward declarations
#include "../core/network_client.h"
#include "../core/thread_safe_queue.h"
#include "../core/network_event.h"
#include "../handlers/session/client_session.h"
#include "../handlers/chat/chat_handler.h"
#include "../handlers/auth/auth_handler.h"
#include "../handlers/rank/rank_handler.h"
#include "../handlers/room/room_handler.h"
#include "../handlers/room/room_structs.h"