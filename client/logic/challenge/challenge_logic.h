#pragma once

#include "../../net/challenge/challenge_sender.h"
#include "../../net/protocol.h"

class ChallengeLogic {
public:
    explicit ChallengeLogic(ChallengeSender& challengeSender);

    // gửi lời mời thách đấu
    void sendChallenge(uint32_t targetUserId, uint32_t roomId);

private:
    ChallengeSender& challengeSender_;
};
