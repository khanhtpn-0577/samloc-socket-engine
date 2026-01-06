#include "player_repository.h"
#include "../../utils/logger.h"
#include <sstream>
#include <iomanip>

PlayerRepository::PlayerRepository(Database& db) : database(db) {}

// Lấy thông tin player. Nếu không có, trả false
bool PlayerRepository::getPlayer(int id, DBPlayer& outPlayer) {
    std::stringstream sql;
    sql << "SELECT player_id, display_name, balance, status FROM players WHERE player_id = " << id;

    auto rows = database.query(sql.str());
    if (rows.empty()) return false;

    try {
        const auto& row = rows[0];

        // ID
        auto it = row.find("player_id");
        if (it == row.end()) return false;
        outPlayer.id = std::stoi(it->second);

        // username

        // display_name
        it = row.find("display_name");
        outPlayer.displayName = (it != row.end() && !it->second.empty()) ? it->second : outPlayer.displayName;

        // balance
        it = row.find("balance");
        outPlayer.balance = (it != row.end() && !it->second.empty()) ? std::stod(it->second) : 0;

        // status
        it = row.find("status");
        outPlayer.status = (it != row.end()) ? it->second : "offline";

        return true;
    } catch (const std::exception& e) {
        Logger::log(LogLevel::ERROR, "PLAYER_REPO", "Failed parsing player " + std::to_string(id) + ": " + e.what());
        return false;
    }
}


// Cộng trừ balance
bool PlayerRepository::updateBalance(int id, double amount) {
    std::stringstream sql;
    sql << std::fixed << std::setprecision(2);
    sql << "UPDATE players SET balance = balance + " << amount << " WHERE player_id = " << id;
    bool success = database.execute(sql.str());
    if (!success) Logger::log(LogLevel::WARNING, "PLAYER_REPO", "Failed to update balance for player " + std::to_string(id));
    return success;
}

// Cập nhật trạng thái player
bool PlayerRepository::updateStatus(int id, const std::string& status) {
    std::stringstream sql;
    sql << "UPDATE players SET status = '" << status << "' WHERE player_id = " << id;
    bool success = database.execute(sql.str());
    if (!success) Logger::log(LogLevel::WARNING, "PLAYER_REPO", "Failed to update status for player " + std::to_string(id));
    return success;
}

// Lấy balance, nếu không có thì fallback 0
double PlayerRepository::getBalance(int id) {
    DBPlayer player;
    if (getPlayer(id, player)) {
        return player.balance;
    } else {
        Logger::log(LogLevel::WARNING, "PLAYER_REPO", "Cannot get balance for player " + std::to_string(id) + ", returning 0");
        return 0.0;
    }
}
