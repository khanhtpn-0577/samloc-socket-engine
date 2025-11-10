#pragma once
#include <string>
#include <sqlite3.h>

// Lớp tiện ích quản lý kết nối SQLite + chạy schema/sample
class Database {
public:
    explicit Database(const std::string& dbPath);
    ~Database();

    // Thực thi câu SQL (CREATE, INSERT, UPDATE,...)
    bool execute(const std::string& sql);

    // Đọc schema từ file SQL và khởi tạo database
    bool initSchemaFromFile(const std::string& schemaFile);

    // Load dữ liệu mẫu từ file sample_data.sql
    bool loadSampleDataFromFile(const std::string& sampleFile);

private:
    sqlite3* db = nullptr;
    std::string readFile(const std::string& path);
};
