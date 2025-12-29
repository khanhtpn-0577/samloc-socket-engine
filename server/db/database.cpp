#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <sqlite3.h>

namespace fs = std::filesystem;

// ------------------------------------------------------
// HELPER: Callback để in dữ liệu bảng ra màn hình
// ------------------------------------------------------
static int debugCallback(void*, int argc, char** argv, char** azColName) {
    std::cout << "      Row: ";
    for (int i = 0; i < argc; i++) {
        std::cout << "[" << azColName[i] << "=" << (argv[i] ? argv[i] : "NULL") << "] ";
    }
    std::cout << "\n";
    return 0;
}
// ------------------------------------------------------
// Constructor
// ------------------------------------------------------
Database::Database(const std::string& dbPath) {
    // std::cout << "\n================ [DATABASE DEBUG START] ================\n";
    // std::cout << "[INFO] DB Path Input: " << dbPath << "\n";

    // Chuyển sang đường dẫn tuyệt đối để dễ debug
    try {
        fs::path absPath = fs::absolute(dbPath);
        std::cout << "[INFO] Absolute Path: " << absPath.string() << "\n";
        
        if (fs::exists(absPath)) {
            std::cout << "[STATUS] File DB DA TON TAI tren o cung.\n";
            std::cout << "[INFO] File size: " << fs::file_size(absPath) << " bytes.\n";
        } else {
            std::cout << "[STATUS] File DB CHUA TON TAI (SQLite se tao moi).\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[WARN] Filesystem error: " << e.what() << "\n";
    }

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        std::cout << "Database opened successfully: " << dbPath << std::endl;
    }

    // std::cout << "--------------------------------------------------------\n";
    // std::cout << "[CHECK 1] Danh sach bang hien co (Schema):\n";
    // const char* listTablesSql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;";
    // char* errMsg = nullptr;
    // int rc = sqlite3_exec(db, listTablesSql, debugCallback, 0, &errMsg);
    // if (rc != SQLITE_OK) {
    //     std::cerr << "[SQL ERROR] " << errMsg << "\n";
    //     sqlite3_free(errMsg);
    // }

    // // 4. DEBUG: Kiểm tra bảng migrations
    // std::cout << "--------------------------------------------------------\n";
    // std::cout << "[CHECK 2] Du lieu bang 'migrations':\n";
    // const char* checkMigSql = "SELECT * FROM migrations;";
    // rc = sqlite3_exec(db, checkMigSql, debugCallback, 0, &errMsg);
    // if (rc != SQLITE_OK) {
    //     // Lỗi này bình thường nếu là DB mới tinh chưa chạy init
    //     std::cout << "[NOTE] Khong doc duoc bang migrations (Co the chua khoi tao): " << errMsg << "\n";
    //     sqlite3_free(errMsg);
    // }
    // std::cout << "================ [DATABASE DEBUG END] ==================\n\n";
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
        std::cerr << "[FILE ERROR] Cannot open file: " << path << std::endl;
        try {
             std::cerr << "[FILE ERROR] Expected at: " << fs::absolute(path).string() << std::endl;
        } catch(...) {}
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

bool Database::applyMigrations(const std::string& dirPath) {
    namespace fs = std::filesystem;

    // Đảm bảo có bảng migrations
    execute(
        "CREATE TABLE IF NOT EXISTS migrations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE, "
        "applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"
    );

    // Collect files và sort theo tên để đảm bảo thứ tự
    std::vector<std::string> sqlFiles;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.path().extension() == ".sql") {
            sqlFiles.push_back(entry.path().string());
        }
    }
    std::sort(sqlFiles.begin(), sqlFiles.end());

    //Duyệt qua từng file .sql theo thứ tự
    for (const auto& filePath : sqlFiles) {
        std::string fileName = fs::path(filePath).filename().string();

        //Kiểm tra migration đã được chạy chưa
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

        //Chạy migration mới
        std::cout << "Applying migration: " << fileName << "\n";
        std::string sql = readFile(filePath);
        char* errMsg = nullptr;

        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Migration failed: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }

        //Ghi nhận migration đã chạy
        std::string insertQuery =
            "INSERT INTO migrations (name) VALUES ('" + fileName + "');";
        if (!execute(insertQuery)) {
            std::cerr << "Failed to record migration: " << fileName << std::endl;
            return false;
        }

        std::cout << "Migration applied: " << fileName << "\n";
    }
    execute("PRAGMA wal_checkpoint(TRUNCATE);");
    return true;
}

