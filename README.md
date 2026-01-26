# Hướng dẫn build và chạy project SamLoc

Tài liệu này hướng dẫn các bước biên dịch và chạy hệ thống game Sâm Lốc gồm 3 phần: khởi tạo cơ sở dữ liệu, server và client. Project sử dụng CMake, yêu cầu môi trường Linux hoặc macOS với trình biên dịch C++ đã được cài đặt.

---

## 1. Khởi tạo cơ sở dữ liệu

Từ thư mục root của project, thực hiện các lệnh sau:

```bash
cd server/db
mkdir build
cd build
cmake ..
make
./init_db
```

Lệnh `init_db` sẽ tạo schema và dữ liệu mẫu cho cơ sở dữ liệu phục vụ server.

---

## 2. Biên dịch và chạy Server

Quay lại thư mục root, sau đó thực hiện:
```bash
cd server
mkdir build
cd build
cmake ..
make
./samloc_server
```

Server sẽ khởi chạy và lắng nghe kết nối client tại cổng mặc định được cấu hình trong mã nguồn.

---

## 3. Biên dịch và chạy Client

Từ thư mục root, thực hiện:

```bash
cd client
mkdir build
cd build
cmake ..
make
./samloc_client
```

Sau khi client được khởi chạy, người dùng có thể đăng nhập và sử dụng các chức năng của game thông qua giao diện đồ họa.

---

## Ghi chú

- Cần đảm bảo server đang chạy trước khi khởi động client.
- Nếu thay đổi mã nguồn, cần chạy lại các bước `cmake` và `make` tương ứng.
