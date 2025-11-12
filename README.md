# Sam Lốc Socket Engine (C++ / SQLite / CMake)

Dự án **Sam Lốc Socket Engine** là nền tảng backend cho trò chơi bài **Sâm Lốc** (và có thể mở rộng cho Mạt Chược).  
Dự án được viết bằng **C++17**, sử dụng **SQLite** làm cơ sở dữ liệu cục bộ, và **CMake** để biên dịch độc lập trên mọi hệ điều hành.

## Yêu cầu môi trường

### Cài đặt CMake

1. Tải bản mới nhất tại trang chính thức:
   [https://cmake.org/download/](https://cmake.org/download/)

2. Các bước cài đặt:
   - Chạy file cài đặt `.msi`
   - Tại bước "Installation Options", chọn *Add CMake to system PATH for all users*

3. Kiểm tra cài đặt:
   - Mở lại VS Code hoặc Terminal
   - Chạy lệnh kiểm tra:
   ```bash
   cmake --version
   ```

## Hướng dẫn chạy project

### Build project bằng CMake

1. Mở Terminal tại thư mục gốc (samloc-socket-engine/) và chạy:
   ```bash
   cmake -B build -S . -G "MinGW Makefiles"
   cmake --build build
   ```

   Lệnh đầu tiên tạo cấu hình build trong thư mục build/,
   lệnh thứ hai biên dịch toàn bộ mã nguồn.

2. Nếu build thành công, sẽ xuất hiện file thực thi:
   ```
   build/samloc.exe
   build/init_db.exe
   ```

3. Khởi tạo hoặc cập nhật database:
   ```bash
   ./build/init_db.exe
   ```

   Lệnh này sẽ tạo file `samloc.db` trong thư mục gốc project hoặc cập nhật nó
   nếu đã tồn tại theo các migration scripts.

### Chạy chương trình

Chạy lệnh sau trong terminal (từ thư mục gốc):
```bash
./build/samloc.exe
```

## Kiểm tra dữ liệu bằng DB Browser for SQLite

### Cài đặt DB Browser

1. Tải bản mới nhất tại:
   [https://sqlitebrowser.org](https://sqlitebrowser.org)

2. Chọn phiên bản tương ứng với Windows (64-bit)

3. Cài đặt như phần mềm bình thường

### Sử dụng DB Browser

1. Mở DB Browser for SQLite

2. Nhấn "Open Database"

3. Chọn file samloc.db trong thư mục gốc project

4. Vào tab "Browse Data"

5. Chọn bảng cần xem (ví dụ: players, rooms, game_results...)

## Quản lý Database Migrations

### Cấu trúc thư mục migrations

Các migration scripts được lưu trữ trong thư mục `server/migrations/`:
```
server/migrations/
├── 001_init_schema.sql
├── 002_add_users_table.sql
├── 003_add_game_sessions.sql
└── ...
```

### Quy tắc Migration

1. **Tạo migration mới**: Khi cần cập nhật database, tạo một file SQL mới với tên theo quy cách:
   - Định dạng: `NNN_description.sql` (NNN là số thứ tự tăng dần)
   - Ví dụ: `004_add_rankings_table.sql`

2. **Nội dung migration**: Viết các lệnh SQL để:
   - Tạo bảng mới (CREATE TABLE)
   - Thêm cột mới (ALTER TABLE ADD COLUMN)
   - Cập nhật cấu trúc cơ sở dữ liệu

3. **Áp dụng migration**: Sau khi tạo file migration mới, chạy lại:
   ```bash
   ./build/init_db.exe
   ```

   Chương trình sẽ tự động phát hiện và thực thi các migration chưa được áp dụng.

4. **Kiểm tra kết quả**: Mở DB Browser for SQLite để xác nhận các thay đổi đã được áp dụng thành công.
