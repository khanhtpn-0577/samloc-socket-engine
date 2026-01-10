#include "private_room_handler.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

PrivateRoomHandler::PrivateRoomHandler(PrivateRoomLogic& logic)
    : privateRoomLogic_(logic) {}

Message PrivateRoomHandler::handleCreatePrivateRoom(
    const Message& incomingMsg
) {
    uint32_t userId = incomingMsg.header.senderId;

    std::cout
        << "[RoomHandler - Start] Handle CREATE_PRIVATE_ROOM for userId="
        << userId << "\n";

    std::string payload;

    try {
        // ==== PARSE PAYLOAD ====
        json j = json::parse(incomingMsg.payload);

        std::string roomName = j.at("name").get<std::string>();
        std::string roomType = j.at("type").get<std::string>();
        int betAmount = j.at("bet").get<int>();

        // ==== CALL LOGIC ====
        payload = privateRoomLogic_.createPrivateRoom(
            userId,
            roomName,
            roomType,
            betAmount
        );

    } catch (const std::exception& e) {
        std::cerr << "[RoomHandler] Payload parse error: "
                  << e.what() << "\n";

        payload = R"({
            "success": false,
            "message": "Invalid payload"
        })";
    }

    // ==== BUILD RESPONSE====
    Message response;
    response.header.messageType =
        static_cast<uint16_t>(MessageType::S_CREATE_PRIVATE_ROOM_RESPONSE);
    response.header.senderId = 0;
    response.header.timestamp =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    response.header.payloadLength = payload.size();
    std::memset(response.header.token, 0, 32);
    response.payload = payload;

    std::cout
        << "[RoomHandler - End] Prepared CREATE_PRIVATE_ROOM_RESPONSE for userId="
        << userId
        << " payload=" << payload
        << "\n";

    return response;
}
