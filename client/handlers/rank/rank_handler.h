#pragma once
#include <vector>
#include <functional>
#include "../../logic/rank/rank_logic.h"
#include "../session/client_session.h"

struct FriendRankInfo {
    uint32_t userId;
    std::string name;
    long long balance;
};

class RankHandler {
public:
    RankHandler(RankLogic& logic, ClientSession& session);

    void requestFriendRank();

    void onFriendRankResponse(const Message& message);

    void setFriendRankCallback(
        std::function<void(const std::vector<FriendRankInfo>&)> cb
    );

private:
    RankLogic& rankLogic_;
    ClientSession& session_;

    std::function<void(const std::vector<FriendRankInfo>&)> friendRankCallback_;
};
