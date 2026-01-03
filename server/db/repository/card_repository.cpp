#include "card_repository.h"
#include "../../utils/logger.h"

CardRepository::CardRepository(Database& db) : database(db) {}

std::vector<DBCard> CardRepository::getAllCards() {
    std::vector<DBCard> cards;
    auto rows = database.query("SELECT * FROM cards ORDER BY order_value ASC");
    
    for (const auto& row : rows) {
        DBCard c;
        try {
            c.id = std::stoi(row.at("card_id"));
            c.suit = row.at("suit");
            c.rank = row.at("rank");
            c.orderValue = std::stoi(row.at("order_value"));
            cards.push_back(c);
        } catch (...) {
            Logger::log(LogLevel::ERROR, "DB", "Error parsing card data");
        }
    }
    return cards;
}