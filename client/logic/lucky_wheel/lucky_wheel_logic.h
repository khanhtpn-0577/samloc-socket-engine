#pragma once
#include <cstdint>
#include "../../net/lucky_wheel/lucky_wheel_sender.h"
#include "../../net/protocol.h"

class LuckyWheelLogic {
public:
    explicit LuckyWheelLogic(LuckyWheelSender& luckyWheelSender);

    void requestSpin(uint32_t userId);

private:
    LuckyWheelSender& luckyWheelSender_;
};
