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
    utils/logger.cpp

    handler/connection/connection_handler.cpp
    handler/session/session_manager.cpp
    handler/chat/chat_handler.cpp
    handler/auth/auth_handler.cpp
    handler/challenge/challenge_handler.cpp
    handler/game/game_handler.cpp 
    logic/chat/chat_logic.cpp

    db/database.cpp
    db/repository/private_chat_repository.cpp
    db/repository/room_repository.cpp   
    db/repository/player_repository.cpp  
    db/repository/card_repository.cpp    
    db/repository/game_repository.cpp   

    logic/auth/auth_logic.cpp
    logic/challenge/challenge_logic.cpp
    logic/game/sam_loc_game.cpp 
)

set(SERVER_HEADERS
    net/server_socket.h
    net/protocol.h
    net/chat/message_sender.h


    handler/connection/connection_handler.h
    handler/session/session_manager.h
    handler/chat/chat_handler.h
    handler/auth/auth_handler.h
    handler/challenge/challenge_handler.h
    handler/game/game_handler.h  
    logic/chat/chat_logic.h
    utils/logger.h

    db/database.h
    db/repository/private_chat_repository.h
    db/repository/room_repository.h   # [NEW]
    db/repository/player_repository.h # [NEW]
    db/repository/card_repository.h   # [NEW]
    db/repository/game_repository.h  

    logic/auth/auth_logic.h
    logic/challenge/challenge_logic.h
    logic/game/sam_loc_game.h  
    logic/game/game_config.h  
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
find_package(OpenSSL)

if(NOT OPENSSL_FOUND)
    message(WARNING "OpenSSL not found - some features may not work")
endif()

target_link_libraries(samloc_server PRIVATE
    Threads::Threads
)

if(OPENSSL_FOUND)
    target_link_libraries(samloc_server PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
    )
endif()

# --- Cấu hình Warnings ---
if (MSVC)
    target_compile_options(samloc_server PRIVATE /W4)
else()
    target_compile_options(samloc_server PRIVATE -Wall -Wextra)
endif()