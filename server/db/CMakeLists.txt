cmake_minimum_required(VERSION 3.10)
project(samloc_db_init)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------
# Source files cho DB init
# --------------------------------------------------
set(DB_SOURCES
    init_db.cpp
    database.cpp
    ../../third_party/sqlite3.c
)

set(DB_HEADERS
    database.h
)

# --------------------------------------------------
# Tạo executable init_db
# --------------------------------------------------
add_executable(init_db
    ${DB_SOURCES}
    ${DB_HEADERS}
)

# --------------------------------------------------
# Include directories
# --------------------------------------------------
target_include_directories(init_db PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party
)

# --------------------------------------------------
# Warnings
# --------------------------------------------------
if (MSVC)
    target_compile_options(init_db PRIVATE /W4)
else()
    target_compile_options(init_db PRIVATE -Wall -Wextra)
endif()
add_library(samloc_db
    database.cpp
    ../../third_party/sqlite3.c
)

target_include_directories(samloc_db
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party
)