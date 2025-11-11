#include "db/database.h"
#include <iostream>
#include <filesystem>

int main() {
    using namespace std;
    namespace fs = std::filesystem;

    cout << "🧱 Initializing Sam Loc Database...\n";

    const std::string dbPath = "samloc.db";
    Database db(dbPath);

    // Nếu DB chưa tồn tại → tạo schema đầy đủ + sample data
    if (!fs::exists(dbPath)) {
        cout << "⚙️ Database not found. Creating new one...\n";

        if (!db.initSchemaFromFile("src/db/schema.sql")) {
            cerr << "❌ Failed to initialize schema.\n";
            return 1;
        }

        if (!db.loadSampleDataFromFile("src/db/sample_data.sql")) {
            cerr << "❌ Failed to load sample data.\n";
            return 1;
        }

        cout << "✅ New database created successfully.\n";
    }
    // Nếu DB đã tồn tại → chạy migration
    else {
        cout << "📦 Database exists. Applying migrations if any...\n";

        if (!db.applyMigrations("src/db/migrations")) {
            cerr << "❌ Migration failed.\n";
            return 1;
        }

        cout << "✅ Migrations applied successfully.\n";
    }

    cout << "🎯 Database ready: " << dbPath << "\n";
    return 0;
}
