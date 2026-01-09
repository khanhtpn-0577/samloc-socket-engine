#include "private_room_logic.h"
#include "../../db/repository/private_room_repository.h"
#include <iostream>

PrivateRoomLogic::PrivateRoomLogic(Database& db)
    : database_(db) {}

std::string PrivateRoomLogic::createPrivateRoom(
    uint32_t createdBy,
    const std::string& roomName,
    const std::string& roomType,
    int betAmount
) {
    std::cout
        << "[RoomLogic - Start] createPrivateRoom by userId="
        << createdBy << "\n";

    try {
        // ==== BASIC VALIDATION ====
        if (roomName.empty()) {
            return R"({"success":false,"message":"Room name is required"})";
        }

        if (roomType != "dat_cuoc" && roomType != "dem_la") {
            return R"({"success":false,"message":"Invalid room type"})";
        }

        if (betAmount < 0) {
            return R"({"success":false,"message":"Invalid bet amount"})";
        }

        RoomPrivateRepository repo(database_);

        PrivateRoomInsertResult result = repo.createPrivateRoom(
            roomName,
            roomType,
            betAmount,
            createdBy
        );

        if (result.roomId == 0) {
            return R"({"success":false,"message":"Failed to create room"})";
        }

        std::string payload = "{";
        payload += "\"success\":true,";
        payload += "\"roomId\":" + std::to_string(result.roomId) + ",";
        payload += "\"roomCode\":" + std::to_string(result.roomCode);
        payload += "}";

        std::cout
            << "[RoomLogic - End] Created roomId="
            << result.roomId << "\n";

        return payload;

    } catch (const std::exception& e) {
        std::cerr
            << "[RoomLogic] Error: "
            << e.what() << "\n";

        return R"({
            "success": false,
            "message": "Internal server error"
        })";
    }
}
