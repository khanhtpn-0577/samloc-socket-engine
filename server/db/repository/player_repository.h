#pragma once
#include "../database.h"
#include "../model_structs.h"
#include <string>

class PlayerRepository {
public:
    explicit PlayerRepository(Database& db);

    bool getPlayer(int id, DBPlayer& outPlayer);
    bool updateBalance(int id, double amount);
    bool updateStatus(int id, const std::string& status);
double getBalance(int id);
private:
    Database& database;
};