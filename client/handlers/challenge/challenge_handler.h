#pragma once

#include <string>
#include <functional>
#include "../../net/protocol.h"
#include "../session/client_session.h"
#include "../../logic/challenge/challenge_logic.h"

class ChallengeHandler {
public:
    explicit ChallengeHandler(ChallengeLogic& challengeLogic, ClientSession& session);

    // GỬI LỜI MỜI THÁCH ĐẤU
    void onSendChallenge(uint32_t targetUserId, uint32_t roomId);

    void onSendChallengeResponse(const Message& message);

    void setChallengeResultCallback(
        std::function<void(bool success, const std::string& message)> cb
    );

    void onChallengeNotification(const Message& message);

    void setChallengeNotifyCallback(
        std::function<void(
            uint32_t fromUserId,
            const std::string& fromUsername,
            uint32_t roomId,
            const std::string& roomType,
            int64_t betAmount
        )> cb
    );

private:
    ClientSession& session_;
    ChallengeLogic& challengeLogic_;

    std::function<void(bool, std::string)> challengeResultCallback_;

    std::function<void(
        uint32_t,
        std::string,
        uint32_t,
        std::string,
        int64_t
    )> challengeNotifyCallback_;
};
