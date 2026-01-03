#pragma once
#include "../database.h"
#include <string>

class GameRepository {
public:
    explicit GameRepository(Database& db);

    int createGame(int roomId, const std::string& gameType);
    void logMove(int gameId, int userId, const std::string& moveType, const std::string& cardJson);
    void endGame(int gameId, int winnerId);
    void saveResult(int gameId, int playerId, int ranking, int cardsLeft, double reward);

private:
    Database& database;
};