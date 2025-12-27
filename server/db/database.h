#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <sqlite3.h>

// Query result row
using QueryRow = std::unordered_map<std::string, std::string>;
using QueryResult = std::vector<QueryRow>;

// Lớp tiện ích quản lý kết nối SQLite + chạy schema/sample
class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();

    // Thực thi câu SQL (CREATE, INSERT, UPDATE,...)
    bool execute(const std::string& sql);

    // Execute query and return results
    QueryResult query(const std::string& sql);

    // Execute prepared statement (safer)
    bool executePrepared(const std::string& sql, const std::vector<std::string>& params);

    // Query with prepared statement
    QueryResult queryPrepared(const std::string& sql, const std::vector<std::string>& params);

    // Get last insert row ID
    int64_t getLastInsertId();

    // Get raw sqlite3 handle
    sqlite3* getHandle() { return db; }

    // Đọc schema từ file SQL và khởi tạo database
    bool initSchemaFromFile(const std::string& schemaFile);

    // Load dữ liệu mẫu từ file sample_data.sql
    bool loadSampleDataFromFile(const std::string& sampleFile);

    //migration
    bool applyMigrations(const std::string& dirPath);

private:
    sqlite3* db = nullptr;
    std::string readFile(const std::string& path);
};
