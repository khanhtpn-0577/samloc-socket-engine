#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>

// ------------------------------------------------------
// Constructor
// ------------------------------------------------------
Database::Database(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "❌ Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        std::cout << "✅ Database opened successfully: " << dbPath << std::endl;
    }
}

// ------------------------------------------------------
// Destructor
// ------------------------------------------------------
Database::~Database() {
    if (db) {
        sqlite3_close(db);
        std::cout << "🧱 Database closed.\n";
    }
}

// ------------------------------------------------------
// Execute generic SQL
// ------------------------------------------------------
bool Database::execute(const std::string& sql) {
    if (!db) return false;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "❌ SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// ------------------------------------------------------
// Read file content
// ------------------------------------------------------
std::string Database::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "❌ Cannot open file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ------------------------------------------------------
// Initialize schema from SQL file
// ------------------------------------------------------
bool Database::initSchemaFromFile(const std::string& schemaFile) {
    std::string sql = readFile(schemaFile);
    if (sql.empty()) {
        std::cerr << "⚠️  Schema file is empty or missing.\n";
        return false;
    }

    std::cout << "📘 Initializing schema from " << schemaFile << "...\n";
    if (execute(sql)) {
        std::cout << "✅ Schema created successfully.\n";
        return true;
    } else {
        std::cerr << "❌ Failed to create schema.\n";
        return false;
    }
}

// ------------------------------------------------------
// Load sample data from SQL file
// ------------------------------------------------------
bool Database::loadSampleDataFromFile(const std::string& sampleFile) {
    std::string sql = readFile(sampleFile);
    if (sql.empty()) {
        std::cerr << "⚠️  Sample data file is empty or missing.\n";
        return false;
    }

    std::cout << "📘 Loading sample data from " << sampleFile << "...\n";
    if (execute(sql)) {
        std::cout << "✅ Sample data loaded successfully.\n";
        return true;
    } else {
        std::cerr << "❌ Failed to load sample data.\n";
        return false;
    }
}
