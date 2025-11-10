#pragma once
#include <string>
#include "../db/database.h"

class Game {
public:
    explicit Game(Database& db);
    void start();
private:
    Database& database;
};
