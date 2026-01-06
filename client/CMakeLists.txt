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
    net/rank/rank_sender.cpp
    core/network_client.cpp

    logic/chat/chat_logic.cpp
    logic/auth/auth_logic.cpp
    logic/rank/rank_logic.cpp
    
    handlers/chat/chat_handler.cpp
    handlers/connection/client_connection_handler.cpp
    handlers/session/client_session.cpp

    handlers/auth/auth_handler.cpp
    handlers/challenge/challenge_handler.cpp
    handlers/rank/rank_handler.cpp
    
    # --- [NEW] Room Logic ---
    handlers/room/room_handler.cpp

    ui/components/button.cpp
    ui/components/text_input.cpp
    ui/components/card_ui.cpp

    ui/screens/login_state.cpp
    ui/screens/lobby_state.cpp
    ui/screens/private_chat_state.cpp
    ui/screens/ranking_state.cpp
    ui/screens/waiting_room_state.cpp
    ui/screens/in_game_state.cpp
    
    # --- [NEW] Room UI ---
    ui/screens/room_list_state.cpp
    ui/screens/in_game_state.cpp
    ui/screens/game_starting_countdown_state.cpp

    ui/game_manager.cpp
)

set(CLIENT_HEADERS
    net/client_socket.h
    net/protocol.h
    net/chat/message_sender.h
    net/auth/auth_sender.h
    net/challenge/challenge_sender.h
    net/rank/rank_sender.h
    core/thread_safe_queue.h
    core/network_event.h
    core/network_client.h

    logic/chat/chat_logic.h
    logic/auth/auth_logic.h
    logic/rank/rank_logic.h
    
    handlers/chat/chat_handler.h
    handlers/connection/client_connection_handler.h
    handlers/session/client_session.h

    handlers/auth/auth_handler.h
    handlers/challenge/challenge_handler.h
    handlers/rank/rank_handler.h
    
    # --- [NEW] Room Headers ---
    handlers/room/room_handler.h
    handlers/room/room_sender.h
    handlers/room/room_structs.h

    ui/game_state.h
    ui/state_context.h
    ui/components/button.h
    ui/components/card_ui.h
    ui/components/text_input.h
    ui/screens/login_state.h
    ui/screens/lobby_state.h
    ui/screens/private_chat_state.h
    ui/screens/ranking_state.h
    ui/screens/in_game_state.h
    
    # --- [NEW] Room UI Headers ---
    ui/screens/room_list_state.h
    ui/screens/in_game_state.h
    ui/components/card_ui.h
    ui/screens/waiting_room_state.h
    ui/screens/game_starting_countdown_state.h

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
# 1. Tìm SFML cài sẵn trên Linux
# =========================
find_package(SFML COMPONENTS graphics window system network REQUIRED)

# =========================
# Include directories
# =========================
target_include_directories(samloc_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SFML_INCLUDE_DIRS} 
)

# =========================
# Threads
# =========================
find_package(Threads REQUIRED)

# =========================
# Link libraries
# =========================
target_link_libraries(samloc_client PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-network
    Threads::Threads
)

# =========================
# Compiler warnings
# =========================
if (MSVC)
    target_compile_options(samloc_client PRIVATE /W4)
else()
    target_compile_options(samloc_client PRIVATE -Wall -Wextra)
endif()