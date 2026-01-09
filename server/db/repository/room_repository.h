#pragma once
#include "../database.h"
#include "../model_structs.h"
#include <string>

class RoomRepository {
public:
    explicit RoomRepository(Database& db);

    bool getRoom(int roomId, DBRoom& outRoom);
    bool addPlayerToRoom(int roomId, int playerId);
    bool updateRoomStatus(int roomId, const std::string& status);
    std::vector<DBRoom> getAllRooms();
    bool removePlayerFromRoom(int roomId, int playerId);
    int getCurrentPlayersInDB(int roomId);
    
private:
    Database& database;
};