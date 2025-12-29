cmake_minimum_required(VERSION 3.10)
project(samloc_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Định nghĩa các file nguồn và header của Server ---
set(SERVER_SOURCES
    main.cpp

    ../third_party/sqlite3.c

    net/server_socket.cpp
    net/chat/message_sender.cpp

    handler/connection/connection_handler.cpp
    handler/session/session_manager.cpp
    handler/chat/chat_handler.cpp

    logic/chat/chat_logic.cpp

    db/database.cpp
    db/repository/private_chat_repository.cpp
)

set(SERVER_HEADERS
    net/server_socket.h
    net/protocol.h
    net/chat/message_sender.h


    handler/connection/connection_handler.h
    handler/session/session_manager.h
    handler/chat/chat_handler.h


    logic/chat/chat_logic.h

    db/database.h
    db/repository/private_chat_repository.h
)

# --- Tạo executable cho Server ---
add_executable(samloc_server
    ${SERVER_SOURCES}
    ${SERVER_HEADERS}
)

# --- Cấu hình Include Directories ---
# Giúp compiler tìm thấy các file header khi include
target_include_directories(samloc_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

# --- Link thư viện ---
# Chuẩn bị sẵn cho thread per client
find_package(Threads REQUIRED)
target_link_libraries(samloc_server PRIVATE
    Threads::Threads
)

# --- Cấu hình Warnings ---
if (MSVC)
    target_compile_options(samloc_server PRIVATE /W4)
else()
    target_compile_options(samloc_server PRIVATE -Wall -Wextra)
endif()
