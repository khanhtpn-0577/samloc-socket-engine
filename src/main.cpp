#include "db/database.h"
#include <iostream>

int main() {
    std::cout << "🚀 Sam Loc Engine – Database Initialization\n";

    // 1️⃣ Kết nối database
    Database db("samloc.db");

    // 2️⃣ Khởi tạo schema
    if (!db.initSchemaFromFile("src/db/schema.sql")) {
        std::cerr << "❌ Schema init failed.\n";
        return 1;
    }

    // 3️⃣ Nạp dữ liệu mẫu
    if (!db.loadSampleDataFromFile("src/db/sample_data.sql")) {
        std::cerr << "❌ Sample data load failed.\n";
        return 1;
    }

    std::cout << "✅ Database + sample data ready.\n";
    std::cout << "💾 File created: samloc.db\n";
    return 0;
}
