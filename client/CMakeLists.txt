cmake_minimum_required(VERSION 3.10)
project(samloc_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# Client source files
# =========================
set(CLIENT_SOURCES
    main.cpp

    net/client_socket.cpp
    net/chat/message_sender.cpp

    logic/chat/chat_logic.cpp
    handlers/chat/chat_handler.cpp
)

set(CLIENT_HEADERS
    net/client_socket.h
    net/protocol.h
    net/chat/message_sender.h

    logic/chat/chat_logic.h
    handlers/chat/chat_handler.h
)

# =========================
# Build executable
# =========================
add_executable(samloc_client
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
)

# =========================
# Include directories
# =========================
target_include_directories(samloc_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =========================
# Threads (Linux / Windows)
# =========================
find_package(Threads REQUIRED)
target_link_libraries(samloc_client PRIVATE Threads::Threads)

# =========================
# Compiler warnings
# =========================
if (MSVC)
    target_compile_options(samloc_client PRIVATE /W4)
else()
    target_compile_options(samloc_client PRIVATE -Wall -Wextra)
endif()
