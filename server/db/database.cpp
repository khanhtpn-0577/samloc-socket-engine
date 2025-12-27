#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sqlite3.h>

// ------------------------------------------------------
// Constructor
// ------------------------------------------------------
Database::Database(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        std::cout << "Database opened successfully: " << dbPath << std::endl;
    }
}

// ------------------------------------------------------
// Destructor
// ------------------------------------------------------
Database::~Database() {
    if (db) {
        sqlite3_close(db);
        std::cout << "Database closed.\n";
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
        std::cerr << "SQL error: " << errMsg << std::endl;
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
        std::cerr << "Cannot open file: " << path << std::endl;
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
        std::cerr << "Schema file is empty or missing.\n";
        return false;
    }

    std::cout << "📘 Initializing schema from " << schemaFile << "...\n";
    if (execute(sql)) {
        std::cout << "Schema created successfully.\n";
        return true;
    } else {
        std::cerr << "Failed to create schema.\n";
        return false;
    }
}

// ------------------------------------------------------
// Load sample data from SQL file
// ------------------------------------------------------
bool Database::loadSampleDataFromFile(const std::string& sampleFile) {
    std::string sql = readFile(sampleFile);
    if (sql.empty()) {
        std::cerr << "Sample data file is empty or missing.\n";
        return false;
    }

    std::cout << "📘 Loading sample data from " << sampleFile << "...\n";
    if (execute(sql)) {
        std::cout << "Sample data loaded successfully.\n";
        return true;
    } else {
        std::cerr << "Failed to load sample data.\n";
        return false;
    }
}

// ------------------------------------------------------
// Query with results
// ------------------------------------------------------
QueryResult Database::query(const std::string& sql) {
    QueryResult result;
    
    if (!db) return result;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    int columnCount = sqlite3_column_count(stmt);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QueryRow row;
        for (int i = 0; i < columnCount; i++) {
            const char* columnName = sqlite3_column_name(stmt, i);
            const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[columnName] = columnValue ? columnValue : "";
        }
        result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return result;
}

// ------------------------------------------------------
// Execute prepared statement
// ------------------------------------------------------
bool Database::executePrepared(const std::string& sql, const std::vector<std::string>& params) {
    if (!db) return false;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); i++) {
        sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "SQL execute error: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    return true;
}

// ------------------------------------------------------
// Query with prepared statement
// ------------------------------------------------------
QueryResult Database::queryPrepared(const std::string& sql, const std::vector<std::string>& params) {
    QueryResult result;
    
    if (!db) return result;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg(db) << std::endl;
        return result;
    }

    // Bind parameters
    for (size_t i = 0; i < params.size(); i++) {
        sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
    }

    int columnCount = sqlite3_column_count(stmt);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        QueryRow row;
        for (int i = 0; i < columnCount; i++) {
            const char* columnName = sqlite3_column_name(stmt, i);
            const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[columnName] = columnValue ? columnValue : "";
        }
        result.push_back(row);
    }

    sqlite3_finalize(stmt);
    return result;
}

// ------------------------------------------------------
// Get last insert row ID
// ------------------------------------------------------
int64_t Database::getLastInsertId() {
    if (!db) return -1;
    return sqlite3_last_insert_rowid(db);
}

bool Database::applyMigrations(const std::string& dirPath) {
    namespace fs = std::filesystem;

    // 1️⃣ Đảm bảo có bảng migrations
    execute(
        "CREATE TABLE IF NOT EXISTS migrations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"
    );

    // 2️⃣ Duyệt qua từng file .sql trong thư mục
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.path().extension() != ".sql") continue;

        std::string fileName = entry.path().filename().string();

        // 3️⃣ Kiểm tra migration đã được chạy chưa
        std::string checkQuery =
            "SELECT COUNT(*) FROM migrations WHERE name = '" + fileName + "';";

        sqlite3_stmt* stmt;
        bool alreadyApplied = false;

        if (sqlite3_prepare_v2(db, checkQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                alreadyApplied = sqlite3_column_int(stmt, 0) > 0;
            }
        }
        sqlite3_finalize(stmt);

        if (alreadyApplied) {
            std::cout << "Skip (already applied): " << fileName << "\n";
            continue;
        }

        // 4️⃣ Chạy migration mới
        std::cout << "Applying migration: " << fileName << "\n";
        std::string sql = readFile(entry.path().string());
        char* errMsg = nullptr;

        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Migration failed: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        // 5️⃣ Ghi nhận migration đã chạy
        std::string insertQuery =
            "INSERT INTO migrations (name) VALUES ('" + fileName + "');";
        execute(insertQuery);

        std::cout << "Migration applied: " << fileName << "\n";
    }

    return true;
}

