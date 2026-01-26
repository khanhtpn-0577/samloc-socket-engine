#include "database.h"
#include <iostream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int, char*[]) {
    // -------------------------------------------------------
    // CẤU HÌNH ĐƯỜNG DẪN (PATH CONFIGURATION)
    // Ngữ cảnh: File thực thi nằm tại server/db/build/
    // -------------------------------------------------------
    
    // In ra thư mục hiện tại để chắc chắn
    fs::path currentPath = fs::current_path();
    std::cout << "--------------------------------------------------\n";
    std::cout << "[INFO] Thu muc chay (CWD): " << currentPath << "\n";

    // 1. Đường dẫn đến DB (Root project: samloc-engine/)
    // server/db/build -> ../../../samloc.db
    std::string dbPath = "../../../samloc.db";
    
    // 2. Đường dẫn đến file SQL (server/db/)
    // server/db/build -> ../schema.sql
    std::string schemaPath = "../schema.sql";
    std::string sampleDataPath = "../sample_data.sql";
    std::string migrationsDir = "../migrations";

    // Debug: In ra đường dẫn tuyệt đối dự kiến để kiểm tra
    try {
        std::cout << "[INFO] DB se duoc luu tai: " << fs::absolute(dbPath).lexically_normal().string() << "\n";
        std::cout << "[INFO] Doc Schema tai:      " << fs::absolute(schemaPath).lexically_normal().string() << "\n";
    } catch (...) {}
    std::cout << "--------------------------------------------------\n\n";

    // -------------------------------------------------------
    // LOGIC KHỞI TẠO
    // -------------------------------------------------------

    // Bước 1: Kết nối DB
    Database db(dbPath);

    // Bước 2: Init Schema
    // Lưu ý: Nếu bảng đã tồn tại, lệnh CREATE TABLE IF NOT EXISTS sẽ bỏ qua, không sao cả.
    if (!db.initSchemaFromFile(schemaPath)) {
        std::cerr << "[FATAL] Khong tim thay file schema.sql hoac loi SQL.\n";
        return 1;
    }

    // Bước 3: Chạy Migrations
    if (!db.applyMigrations(migrationsDir)) {
        std::cerr << "[FATAL] Loi khi chay Migrations.\n";
        return 1;
    }

    // Bước 4: Load Sample Data (Bật lên nếu cần test)
    std::cout << "Nap du lieu mau...\n";
    if (!db.loadSampleDataFromFile(sampleDataPath)) {
        std::cerr << "[WARN] Khong load duoc sample data.\n";
    }

    std::cout << "\n=== [SUCCESS] DB INIT HOAN TAT ===\n";
    return 0;
}