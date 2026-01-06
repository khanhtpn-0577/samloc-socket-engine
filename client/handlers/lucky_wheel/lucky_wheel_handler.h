#pragma once

#include <functional>
#include <string>

#include "../../logic/lucky_wheel/lucky_wheel_logic.h"
#include "../session/client_session.h"

// ================= DATA =================

struct LuckyWheelResult {
    bool success;
    std::string message;
    std::string rewardName;
    long long rewardAmount;
    long long newBalance;
};

// ================= HANDLER =================

class LuckyWheelHandler {
public:
    LuckyWheelHandler(LuckyWheelLogic& logic, ClientSession& session);

    // UI → Handler
    void requestSpin();

    // Network → Handler
    void onSpinResponse(const Message& message);

    // Handler → UI
    void setSpinResultCallback(
        std::function<void(const LuckyWheelResult&)> cb
    );

private:
    LuckyWheelLogic& luckyWheelLogic_;
    ClientSession& session_;

    std::function<void(const LuckyWheelResult&)> spinResultCallback_;
};
