#include "private_room_repository.h"
#include <stdexcept>
#include <random>

RoomPrivateRepository::RoomPrivateRepository(Database& db)
    : database_(db) {}

// helper tạo room_code ngẫu nhiên 6 chữ số
static uint32_t generateRoomCode() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dist(100000, 999999);
    return dist(gen);
}

PrivateRoomInsertResult
RoomPrivateRepository::createPrivateRoom(
    const std::string& roomName,
    const std::string& roomType,
    int betAmount,
    uint32_t createdBy
) {
    uint32_t roomCode = generateRoomCode();

    std::string sql =
        "INSERT INTO rooms ("
        "room_code, room_name, room_type, bet_amount, "
        "is_private, status, max_players, current_players, created_by"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    database_.executePrepared(
        sql,
        {
            std::to_string(roomCode),
            roomName,
            roomType,
            std::to_string(betAmount),
            "1",                // is_private = true
            "waiting",          // status
            "4",                // max_players
            "0",                // current_players
            std::to_string(createdBy)
        }
    );

    uint32_t roomId =
        static_cast<uint32_t>(database_.getLastInsertId());

    if (roomId == 0) {
        throw std::runtime_error("Failed to insert private room");
    }

    return { roomId, roomCode };
}
