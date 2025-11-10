cmake_minimum_required(VERSION 3.16)
project(samloc-socket-engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Thêm thư mục con ---
add_subdirectory(src)

# --- Tạo executable chính ---
add_executable(samloc
    src/main.cpp
    third_party/sqlite3.c
)

# --- Include third_party (sqlite3.h) ---
target_include_directories(samloc PRIVATE third_party)

# --- Link các module ---
target_link_libraries(samloc
    PRIVATE
        db
        game
)

# --- Compiler warnings ---
if (MSVC)
    target_compile_options(samloc PRIVATE /W4)
else()
    target_compile_options(samloc PRIVATE -Wall -Wextra -Wpedantic)
endif()
