#include "db/database.h"
#include <iostream>
#include <filesystem>

int main() {
    using namespace std;
    namespace fs = std::filesystem;

    cout << "Initializing Sam Loc Database...\n";

    const std::string dbPath = "samloc.db";
    bool dbExists = fs::exists(dbPath) && fs::file_size(dbPath) > 0;

    Database db(dbPath);

    if (!dbExists) {
        cout << "Database not found or empty. Creating new one...\n";

        if (!db.initSchemaFromFile("src/db/schema.sql")) {
            cerr << "Failed to initialize schema.\n";
            return 1;
        }

        if (!db.loadSampleDataFromFile("src/db/sample_data.sql")) {
            cerr << "Failed to load sample data.\n";
            return 1;
        }

        if (!db.applyMigrations("src/db/migrations")) {
            cerr << "Failed to apply migrations after init.\n";
            return 1;
        }

        cout << "New database created successfully.\n";
    } else {
        cout << "Database exists. Applying only new migrations...\n";

        if (!db.applyMigrations("src/db/migrations")) {
            cerr << "Migration failed.\n";
            return 1;
        }

        cout << "New migrations applied successfully.\n";
    }

    cout << "Database ready: " << dbPath << "\n";
    return 0;
}
