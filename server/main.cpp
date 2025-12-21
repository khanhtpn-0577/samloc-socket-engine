#include <iostream>
#include <csignal>

#include "net/server_socket.h"
#include "handler/connection/connection_handler.h"

int main() {
    constexpr int SERVER_PORT = 5000;

    std::cout << "=== Samloc Server ===" << std::endl;

    // 1. Create server socket
    ServerSocket server(SERVER_PORT);

    // 2. Start listening
    if (!server.listen()) {
        std::cerr << "Failed to start server on port "
                  << SERVER_PORT << std::endl;
        return 1;
    }

    std::cout << "Server started successfully." << std::endl;

    // 3. Accept loop
    while (true) {
        int clientFd = server.accept();

        if (clientFd < 0) {
            std::cerr << "Accept failed, continue..." << std::endl;
            continue;
        }

        // 4. Handle one client (single-thread mode)
        ConnectionHandler handler(clientFd);
        handler.handle();

        // NOTE:
        // Hiện tại server xử lý tuần tự (blocking).
        // Khi scale:
        //  - thread per client
        //  - hoặc epoll / select
    }

    return 0;
}
