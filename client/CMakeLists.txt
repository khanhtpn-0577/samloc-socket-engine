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
    net/auth/auth_sender.cpp
    net/challenge/challenge_sender.cpp
    core/network_client.cpp

    logic/chat/chat_logic.cpp
    logic/auth/auth_logic.cpp
    
    handlers/chat/chat_handler.cpp
    handlers/connection/client_connection_handler.cpp
    handlers/session/client_session.cpp

    handlers/auth/auth_handler.cpp
    handlers/challenge/challenge_handler.cpp

    ui/components/button.cpp
    ui/components/text_input.cpp
    ui/screens/login_state.cpp
    ui/screens/lobby_state.cpp
    ui/screens/private_chat_state.cpp
    ui/game_manager.cpp
)

set(CLIENT_HEADERS
    net/client_socket.h
    net/protocol.h
    net/chat/message_sender.h
    net/auth/auth_sender.h
    net/challenge/challenge_sender.h
    core/thread_safe_queue.h
    core/network_event.h
    core/network_client.h

    logic/chat/chat_logic.h
    logic/auth/auth_logic.h
    
    handlers/chat/chat_handler.h
    handlers/connection/client_connection_handler.h
    handlers/session/client_session.h

    handlers/auth/auth_handler.h
    handlers/challenge/challenge_handler.h

    ui/game_state.h
    ui/state_context.h
    ui/components/button.h
    ui/components/text_input.h
    ui/screens/login_state.h
    ui/screens/lobby_state.h
    ui/screens/private_chat_state.h
    ui/game_manager.h
)

# =========================
# Build executable
# =========================
add_executable(samloc_client
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
)

# =========================
# SFML via FetchContent
# =========================
include(FetchContent)

set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_EXAMPLES FALSE)
set(SFML_BUILD_DOC FALSE)
set(SFML_BUILD_TEST_SUITE FALSE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)

FetchContent_MakeAvailable(SFML)

# =========================
# Include directories
# =========================
target_include_directories(samloc_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =========================
# Threads
# =========================
find_package(Threads REQUIRED)
# FetchContent_MakeAvailable already provides SFML, no need for find_package
target_link_libraries(samloc_client PRIVATE sfml-graphics sfml-window sfml-system sfml-network)

# =========================
# Link libraries (CHỈ LINK 1 LẦN)
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
