#include "lucky_wheel_handler.h"
#include <iostream>

LuckyWheelHandler::LuckyWheelHandler(
    LuckyWheelLogic& logic,
    ClientSession& session
)
    : luckyWheelLogic_(logic),
      session_(session) {}

// ================= CALLBACK =================

void LuckyWheelHandler::setSpinResultCallback(
    std::function<void(const LuckyWheelResult&)> cb
) {
    spinResultCallback_ = std::move(cb);
}

// ================= REQUEST =================

void LuckyWheelHandler::requestSpin() {
    std::cout << "[LuckyWheelHandler] Requesting spin...\n";

    if (!session_.isLoggedIn()) {
        std::cerr << "[LuckyWheelHandler] User not logged in\n";
        return;
    }

    luckyWheelLogic_.requestSpin(session_.userId());
}

// ================= RESPONSE =================

void LuckyWheelHandler::onSpinResponse(const Message& message) {
    const std::string& payload = message.payload;

    LuckyWheelResult result{};
    result.rewardAmount = 0;

    // ===== success =====
    size_t pos = payload.find("\"success\":");
    if (pos == std::string::npos) {
        std::cerr << "[LuckyWheelHandler] Invalid payload\n";
        return;
    }

    size_t successStart = pos + 10;
    result.success =
        payload.substr(successStart, 4) == "true";

    // ===== message =====
    size_t msgPos = payload.find("\"message\":\"");
    if (msgPos != std::string::npos) {
        size_t start = msgPos + 11;
        size_t end = payload.find("\"", start);
        result.message = payload.substr(start, end - start);
    }

    // ===== reward =====
    size_t rewardPos = payload.find("\"reward\":");
    if (rewardPos != std::string::npos &&
        payload.find("null", rewardPos) == std::string::npos) {

        size_t namePos = payload.find("\"name\":\"", rewardPos);
        if (namePos != std::string::npos) {
            size_t start = namePos + 8;
            size_t end = payload.find("\"", start);
            result.rewardName = payload.substr(start, end - start);
        }

        size_t amountPos = payload.find("\"amount\":", rewardPos);
        if (amountPos != std::string::npos) {
            size_t start = amountPos + 9;
            size_t end = payload.find_first_of(",}", start);
            result.rewardAmount =
                std::stoll(payload.substr(start, end - start));
        }
    }

    // ===== newBalance =====
    size_t balPos = payload.find("\"newBalance\":");
    if (balPos != std::string::npos) {
        size_t start = balPos + 13;
        size_t end = payload.find_first_of(",}", start);
        result.newBalance =
            std::stoll(payload.substr(start, end - start));

        //CẬP NHẬT SESSION
        session_.setBalance(result.newBalance);
    }

    std::cout << "[LuckyWheelHandler] Spin result: "
              << result.rewardName
              << ", newBalance=" << result.newBalance
              << "\n";

    if (spinResultCallback_) {
        spinResultCallback_(result);
    }
}
