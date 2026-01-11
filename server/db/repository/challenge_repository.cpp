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
