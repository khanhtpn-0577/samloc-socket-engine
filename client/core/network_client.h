#pragma once

#include <atomic>
#include <thread>
#include <string>
#include "thread_safe_queue.h"
#include "network_event.h"
#include "../net/client_socket.h"
#include "../net/chat/message_sender.h"
#include "../net/auth/auth_sender.h"
#include "../net/challenge/challenge_sender.h"
#include "../net/friend/friend_sender.h"
#include "../handlers/session/client_session.h"
#include "../net/rank/rank_sender.h"

struct NetworkConfig {
    std::string serverIp;
    int serverPort;
};

class NetworkClient {
public:
    NetworkClient(const NetworkConfig& cfg, ThreadSafeQueue<NetworkEvent>& queue, ClientSession& session);
    ~NetworkClient();

    bool start();
    void stop();

    MessageSender& chatSender();
    AuthSender& authSender();
    ChallengeSender& challengeSender();
    RankSender& rankSender();
    FriendSender& friendSender();
    ClientSocket& socket();

private:
    void run();
    void pushDisconnect(const std::string& reason);

    NetworkConfig config_;
    ThreadSafeQueue<NetworkEvent>& queue_;
    ClientSocket socket_;
    ClientSession& session_;
    MessageSender chatSender_;
    AuthSender authSender_;
    ChallengeSender challengeSender_;
    RankSender rankSender_;
    FriendSender friendSender_;
    std::atomic<bool> running_;
    std::thread worker_;
};
