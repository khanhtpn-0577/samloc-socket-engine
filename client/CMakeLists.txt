# --- Định nghĩa các file nguồn và header của Client ---
set(CLIENT_SOURCES
    main.cpp
    network/client_socket.cpp
    network/message_sender.cpp
    handlers/chat_handler.cpp
)

set(CLIENT_HEADERS
    network/client_socket.h
    network/message_sender.h
    network/protocol.h
    handlers/chat_handler.h
)

# --- Tạo executable cho Client ---
add_executable(samloc_client
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
)

# --- Cấu hình Include Directories ---
# Giúp compiler tìm thấy các file header khi include
target_include_directories(samloc_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Link thư viện ---
# Vì bạn dùng std::mutex trong message_sender.h nên cần link thư viện Threads
find_package(Threads REQUIRED)
target_link_libraries(samloc_client PRIVATE
    Threads::Threads
)

# --- Cấu hình Warnings (kế thừa từ root hoặc set riêng) ---
if (MSVC)
    target_compile_options(samloc_client PRIVATE /W4)
else()
    target_compile_options(samloc_client PRIVATE -Wall -Wextra)
endif()