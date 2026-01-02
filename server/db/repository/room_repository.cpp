#include "room_repository.h"
#include <sstream>

RoomRepository::RoomRepository(Database& db) : database(db) {}

bool RoomRepository::getRoom(int roomId, DBRoom& out) {
    auto rows = database.query("SELECT * FROM rooms WHERE room_id=" + std::to_string(roomId));
    if (rows.empty()) return false;
    auto& r = rows[0];
    out.id = std::stoi(r.at("room_id"));
    out.type = r.at("room_type");
    out.roomName = r.at("room_name");
    out.betAmount = std::stod(r.at("bet_amount"));
    out.maxPlayers = std::stoi(r.at("max_players"));
    out.currentPlayers = std::stoi(r.at("current_players"));
    out.status = r.at("status");
    return true;
}

bool RoomRepository::addPlayerToRoom(int roomId, int playerId) {
    if (!database.execute("INSERT OR IGNORE INTO room_players(room_id,player_id) VALUES("
        + std::to_string(roomId) + "," + std::to_string(playerId) + ")")) return false;
    database.execute("UPDATE rooms SET current_players = current_players + 1 WHERE room_id=" + std::to_string(roomId));
    return true;
}

bool RoomRepository::removePlayerFromRoom(int roomId, int playerId) {
    if (!database.execute("DELETE FROM room_players WHERE room_id=" + std::to_string(roomId)
        + " AND player_id=" + std::to_string(playerId))) return false;
    database.execute("UPDATE rooms SET current_players = MAX(current_players - 1,0) WHERE room_id=" + std::to_string(roomId));
    return true;
}

bool RoomRepository::updateRoomStatus(int roomId, const std::string& s) {
    return database.execute("UPDATE rooms SET status='" + s + "' WHERE room_id=" + std::to_string(roomId));
}

std::vector<DBRoom> RoomRepository::getAllRooms() {
    std::vector<DBRoom> v;
    auto rows = database.query("SELECT * FROM rooms ORDER BY room_id");
    for (auto& r : rows) {
        DBRoom d;
        d.id = std::stoi(r.at("room_id"));
        d.type = r.at("room_type");
        d.betAmount = std::stod(r.at("bet_amount"));
        d.maxPlayers = std::stoi(r.at("max_players"));
        d.currentPlayers = std::stoi(r.at("current_players"));
        d.roomName = r.at("room_name");
        d.status = r.at("status");
        v.push_back(d);
    }
    return v;
}
