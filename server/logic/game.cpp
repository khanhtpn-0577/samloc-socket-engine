#include "game.h"
#include <iostream>

Game::Game(Database& db) : database(db) {}

void Game::start() {
    std::cout << "🎮 Starting Sâm Lốc game logic...\n";

    const std::string createTable = R"(
        CREATE TABLE IF NOT EXISTS players (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT,
            score INTEGER
        );
    )";

    if (database.execute(createTable))
        std::cout << "✅ Table created successfully.\n";
    else
        std::cerr << "❌ Failed to create table.\n";
}
