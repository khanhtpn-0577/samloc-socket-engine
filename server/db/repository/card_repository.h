#pragma once
#include "../database.h"
#include "../model_structs.h"
#include <vector>

class CardRepository {
public:
    explicit CardRepository(Database& db);

    std::vector<DBCard> getAllCards();

private:
    Database& database;
};