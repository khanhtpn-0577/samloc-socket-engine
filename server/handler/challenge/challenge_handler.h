#pragma once

#include "../../net/protocol.h"
#include "../../logic/challenge/challenge_logic.h"

class ChallengeHandler {
public:
    explicit ChallengeHandler(ChallengeLogic& logic);

    Message handleSendChallenge(const Message& incomingMsg);

private:
    ChallengeLogic& challengeLogic_;
};
