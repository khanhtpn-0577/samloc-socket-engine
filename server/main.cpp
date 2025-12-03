#include "db/database.h"
#include "network/server_socket.h"
#include "network/connection_handler.h"
#include <iostream>
#include <thread>
#include <unistd.h>

int main() {
    std::cout << "Sam Loc Engine – Server Initialization\n";

    // Kết nối database
    Database db("samloc.db");

    // Khởi tạo server socket
    int port = 5000;
    ServerSocket server(port);

    if (!server.listen()) {
        std::cerr << "Failed to start server on port " << port << "\n";
        return 1;
    }

    std::cout << " Server started on port " << port << "\n";
    std::cout << " Listening for client connections...\n";

    //  Lặp chấp nhận kết nối từ client
    while (true) {
        int clientSocket = server.accept(); //lang nghe va tao socket moi lam viec voi client
        
        if (clientSocket != -1) {
            std::cout << "Client connected with fd=" << clientSocket << "\n";
            
            // Xử lý client trong thread riêng
            std::thread clientThread([clientSocket]() {
                ConnectionHandler handler(clientSocket);
                handler.handle();
            });
            
            clientThread.detach();
        } else {
            std::cerr << " Failed to accept client connection\n";
            sleep(1);  // Tránh spin loop
        }
    }

    return 0;
}
