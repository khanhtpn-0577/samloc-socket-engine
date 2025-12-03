add_subdirectory(db)
add_subdirectory(game)
add_subdirectory(network)

# Main executable
add_executable(samloc
    main.cpp
    ../third_party/sqlite3.c
)

# Link libraries
target_link_libraries(samloc
    PRIVATE
        db
        game
        network
        pthread
)

# Include directories
target_include_directories(samloc
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

# Include third_party for all subdirectories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
