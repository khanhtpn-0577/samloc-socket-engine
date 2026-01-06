#pragma once

#include "../../net/protocol.h"
#include "../../logic/lucky_wheel/lucky_wheel_logic.h"

class LuckyWheelHandler {
public:
    explicit LuckyWheelHandler(LuckyWheelLogic& logic);

    Message handleSpinRequest(const Message& incomingMsg);

private:
    LuckyWheelLogic& luckyWheelLogic_;
};
