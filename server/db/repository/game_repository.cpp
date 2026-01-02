#include "game_repository.h"
#include "../../utils/logger.h"
#include <sstream>

GameRepository::GameRepository(Database& db) : database(db) {}

int GameRepository::createGame(int roomId, const std::string& gameType) {
    std::stringstream sql;
    sql << "INSERT INTO games (room_id, game_type, start_time, status) "
        << "VALUES (" << roomId << ", '" << gameType << "', CURRENT_TIMESTAMP, 'playing')";
    
    if (database.execute(sql.str())) {
        int gameId = (int)database.getLastInsertId();
        Logger::log(LogLevel::INFO, "DB", "Created Game ID: " + std::to_string(gameId));
        return gameId;
    }
    return -1;
}

void GameRepository::logMove(int gameId, int userId, const std::string& moveType, const std::string& cardJson) {
    std::stringstream sql;
    sql << "INSERT INTO game_moves (game_id, user_id, move_type, card_played) "
        << "VALUES (" << gameId << ", " << userId << ", '" << moveType << "', '" << cardJson << "')";
    database.execute(sql.str());
}

void GameRepository::endGame(int gameId, int winnerId) {
    std::stringstream sql;
    sql << "UPDATE games SET end_time = CURRENT_TIMESTAMP, status = 'finished', winner_id = " << winnerId
        << " WHERE game_id = " << gameId;
    database.execute(sql.str());
}

void GameRepository::saveResult(int gameId, int playerId, int ranking, int cardsLeft, double reward) {
    std::stringstream sql;
    sql << "INSERT INTO game_results (game_id, player_id, ranking, card_left_count, reward) "
        << "VALUES (" << gameId << ", " << playerId << ", " << ranking << ", " << cardsLeft << ", " << reward << ")";
    database.execute(sql.str());
}