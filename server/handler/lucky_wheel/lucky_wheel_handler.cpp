#include "lucky_wheel_handler.h"
#include <iostream>
#include <chrono>
#include <cstring>

LuckyWheelHandler::LuckyWheelHandler(LuckyWheelLogic& logic)
    : luckyWheelLogic_(logic) {}

Message LuckyWheelHandler::handleSpinRequest(
    const Message& incomingMsg
) {
    uint32_t userId = incomingMsg.header.senderId;

    std::cout << "[LuckyWheel Handler - Start] Handle LUCKY_WHEEL_SPIN_REQUEST for userId="
              << userId << "\n";

    // Gọi logic xử lý quay thưởng
    std::string payload = luckyWheelLogic_.spin(userId);

    Message response;
    response.header.messageType =
        static_cast<uint16_t>(MessageType::LUCKY_WHEEL_SPIN_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = payload.size();
    std::memset(response.header.token, 0, 32);
    response.payload = payload;

    std::cout << "[LuckyWheel Handler - End] Prepared LUCKY_WHEEL_SPIN_RESPONSE for userId="
              << userId << " with payload: " << payload << "\n";

    return response;
}
