#include "player_repository.h"
#include "../../utils/logger.h"
#include <sstream>

PlayerRepository::PlayerRepository(Database& db) : database(db) {}

bool PlayerRepository::getPlayer(int id, DBPlayer& outPlayer) {
    std::stringstream sql;
    sql << "SELECT player_id, username, balance, status FROM players WHERE player_id = " << id;
    
    auto rows = database.query(sql.str());
    if (rows.empty()) return false;

    try {
        const auto& row = rows[0];
        outPlayer.id = std::stoi(row.at("player_id"));
        outPlayer.username = row.at("username");
        outPlayer.balance = std::stod(row.at("balance"));
        outPlayer.status = row.at("status");
        return true;
    } catch (...) {
        return false;
    }
}

bool PlayerRepository::updateBalance(int id, double amount) {
    std::stringstream sql;
    sql << "UPDATE players SET balance = balance + " << amount << " WHERE player_id = " << id;
    return database.execute(sql.str());
}

bool PlayerRepository::updateStatus(int id, const std::string& status) {
    std::stringstream sql;
    sql << "UPDATE players SET status = '" << status << "' WHERE player_id = " << id;
    return database.execute(sql.str());
}