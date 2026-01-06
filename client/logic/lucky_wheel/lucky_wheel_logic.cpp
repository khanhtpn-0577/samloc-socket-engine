#include "lucky_wheel_logic.h"
#include <iostream>

LuckyWheelLogic::LuckyWheelLogic(LuckyWheelSender& luckyWheelSender)
    : luckyWheelSender_(luckyWheelSender) {}

void LuckyWheelLogic::requestSpin(uint32_t userId) {
    if (userId == 0) {
        std::cerr << "[LuckyWheelLogic] Invalid userId\n";
        return;
    }

    std::string payload =
        "{\"userId\":" + std::to_string(userId) + "}";

    if (!luckyWheelSender_.sendMessage(
            MessageType::LUCKY_WHEEL_SPIN_REQUEST,
            payload
        )) {
        std::cerr << "[LuckyWheelLogic] Failed to send spin request\n";
    } else {
        std::cout << "[LuckyWheelLogic] Spin request sent for userId="
                  << userId << "\n";
    }
}
