#pragma once

#include "../../net/protocol.h"
#include "../../logic/rank/rank_logic.h"

class RankHandler {
public:
    explicit RankHandler(RankLogic& logic);

    Message handleFriendRankRequest(const Message& incomingMsg);

private:
    RankLogic& rankLogic_;
};
