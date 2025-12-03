add_library(network
    server_socket.cpp
    connection_handler.cpp
)

target_include_directories(network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(network
    PUBLIC
        cxx_std_17
)
