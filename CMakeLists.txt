cmake_minimum_required(VERSION 3.16)
project(samloc_socket_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(server)
add_subdirectory(client)

# =============================
# Executable khởi tạo database (init_db)
# =============================
add_executable(init_db
    server/init_db.cpp
)

target_link_libraries(init_db
    PRIVATE
        samloc_db
)

# --- Compiler warnings ---
if (MSVC)
    target_compile_options(init_db PRIVATE /W4)
else()
    target_compile_options(init_db PRIVATE -Wall -Wextra -Wpedantic)
endif()
