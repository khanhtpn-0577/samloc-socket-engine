#pragma once
#include "../../net/rank/rank_sender.h"

class RankLogic {
public:
    explicit RankLogic(RankSender& rankSender);
    void requestFriendRank(uint32_t userId);

private:
    RankSender& rankSender_;
};
