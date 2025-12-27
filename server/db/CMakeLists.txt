add_library(samloc_db
    database.cpp
    ${CMAKE_SOURCE_DIR}/third_party/sqlite3.c
)

target_include_directories(samloc_db
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party
)