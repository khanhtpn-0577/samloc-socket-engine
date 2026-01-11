#include "network_client.h"

#include <iostream>

NetworkClient::NetworkClient(
    const NetworkConfig& cfg,
    ThreadSafeQueue<NetworkEvent>& queue,
    ClientSession& session
)
    : config_(cfg),
      queue_(queue),
      socket_(cfg.serverIp, cfg.serverPort),
      session_(session),                       //dùng session bên ngoài
      chatSender_(socket_, session_),          
      authSender_(socket_, 0, ""),          
      challengeSender_(socket_, session_),
      rankSender_(socket_, session_),
      roomSender_(socket_, session_), 
      friendSender_(socket_),
      luckyWheelSender_(socket_, session_),
      running_(false) {}

NetworkClient::~NetworkClient() {
    stop();
}

bool NetworkClient::start() {
    if (!socket_.connect()) {
        pushDisconnect("Failed to connect");
        return false;
    }
    running_ = true;
    worker_ = std::thread(&NetworkClient::run, this);
    return true;
}

void NetworkClient::stop() {
    running_ = false;
    socket_.disconnect();
    if (worker_.joinable()) {
        worker_.join();
    }
}

MessageSender& NetworkClient::chatSender() {
    return chatSender_;
}

AuthSender& NetworkClient::authSender() {
    return authSender_;
}

ChallengeSender& NetworkClient::challengeSender() {
    return challengeSender_;
}

RankSender& NetworkClient::rankSender() {
    return rankSender_;
}

RoomSender& NetworkClient::roomSender() {
    return roomSender_;
}

FriendSender& NetworkClient::friendSender() {
    return friendSender_;
}


LuckyWheelSender& NetworkClient::luckyWheelSender() {
    return luckyWheelSender_;
}

ClientSocket& NetworkClient::socket() {
    return socket_;
}

void NetworkClient::pushDisconnect(const std::string& reason) {
    NetworkEvent ev;
    ev.payload = DisconnectEvent{reason};
    queue_.push(std::move(ev));
}

void NetworkClient::run() {
    while (running_) {
        if (!socket_.isConnected()) {
            pushDisconnect("Socket closed");
            running_ = false;
            break;
        }

        Message msg = socket_.receiveMessage();

        if (!socket_.isConnected()) {
            pushDisconnect("Server disconnected");
            running_ = false;
            break;
        }

        NetworkEvent ev;
        ev.payload = RawMessageEvent{msg};
        queue_.push(std::move(ev));
    }
}
