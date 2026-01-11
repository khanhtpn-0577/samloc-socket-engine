#include "challenge_repository.h"
#include <iostream>

ChallengeRepository::ChallengeRepository(Database& db)
    : database_(db) {}

bool ChallengeRepository::isRoomOwner(
    uint32_t userId,
    uint32_t roomId
) {

    std::cout
        << "[ChallengeRepository] Check room owner:"
        << " roomId=" << roomId
        << ", created_by(userId)=" << userId
        << "\n";


    std::string sql =
        "SELECT 1 FROM rooms "
        "WHERE room_id = ? AND created_by = ? LIMIT 1;";

    
    QueryResult rows =
        database_.queryPrepared(
            sql,
            { std::to_string(roomId), std::to_string(userId) }
        );

    return !rows.empty();
}

bool ChallengeRepository::getRoomChallengeInfo(
    uint32_t roomId,
    RoomChallengeInfo& out
) {
    std::string sql =
        "SELECT room_type, bet_amount "
        "FROM rooms "
        "WHERE room_id = ? LIMIT 1;";

    QueryResult rows =
        database_.queryPrepared(
            sql,
            { std::to_string(roomId) }
        );

    if (rows.empty()) return false;

    out.roomType  = rows[0].at("room_type");
    out.betAmount = std::stoll(rows[0].at("bet_amount"));

    std::cout
        << "[ChallengeRepository] Room info:"
        << " roomId=" << roomId
        << " type=" << out.roomType
        << " bet=" << out.betAmount
        << "\n";

    return true;
}

bool ChallengeRepository::getUsernameByUserId(
    uint32_t userId,
    std::string& username
) {
    std::string sql =
        "SELECT username FROM players "
        "WHERE player_id = ? LIMIT 1;";

    QueryResult rows =
        database_.queryPrepared(
            sql,
            { std::to_string(userId) }
        );

    if (rows.empty()) return false;

    username = rows[0].at("username");

    std::cout
        << "[ChallengeRepository] sender username="
        << username << "\n";

    return true;
}


