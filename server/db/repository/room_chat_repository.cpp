#include "room_chat_repository.h"

RoomChatRepository::RoomChatRepository(Database& db)
    : database(db) {}

std::vector<uint32_t>
RoomChatRepository::getRoomMemberIds(uint32_t roomId) {
    std::vector<uint32_t> result;

    std::string sql =
        "SELECT player_id FROM room_players WHERE room_id = ?;";

    QueryResult rows =
        database.queryPrepared(sql, { std::to_string(roomId) });

    for (const auto& row : rows) {
        result.push_back(
            static_cast<uint32_t>(
                std::stoul(row.at("player_id"))
            )
        );
    }

    return result;
}
