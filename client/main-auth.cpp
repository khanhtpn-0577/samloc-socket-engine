// #include <iostream>
// #include <string>
// #include <thread>   // Thư viện cho luồng
// #include <atomic>   // Thư viện cho biến cờ hiệu an toàn giữa các luồng

// #include "net/client_socket.h"
// #include "net/chat/message_sender.h"
// #include "logic/chat/chat_logic.h"
// #include "handlers/chat/chat_handler.h"
// #include "handlers/connection/client_connection_handler.h"
// #include "handlers/auth/auth_handler.h"
// #include "handlers/challenge/challenge_handler.h"
// #include "net/protocol.h"
// #include <SFML/Graphics.hpp>
// #include "core/network_client.h"
// #include "core/thread_safe_queue.h"
// #include "core/network_event.h"
// #include "handlers/session/client_session.h"
// #include "ui/game_manager.h"
// #include "ui/state_context.h"

// // Biến cờ hiệu để kiểm soát vòng lặp, atomic đảm bảo an toàn thread
// std::atomic<bool> isRunning(true);

// // Hàm lắng nghe tin nhắn (sẽ chạy ở luồng riêng)
// void receiveTask(ClientSocket* socket, ClientConnectionHandler* connHandler) {
//     while (isRunning) {
//         try {
//             // Giả sử receiveMessage là blocking (chờ đến khi có tin mới)
//             // Cần xử lý trường hợp socket bị đóng hoặc lỗi để break vòng lặp
//             Message serverMsg = socket->receiveMessage();
            
//             // In dấu xuống dòng để không bị đè lên dòng "You: " đang nhập dở
//             std::cout << "\n"; 
//             connHandler->handleMessage(serverMsg);
            
//             // In lại prompt nhập liệu cho đẹp (optional)
//             std::cout << "\nYou: " << std::flush; 
//         } catch (const std::exception& e) {
//             // Nếu socket lỗi hoặc ngắt kết nối
//             if (isRunning) std::cerr << "\nConnection lost: " << e.what() << "\n";
//             isRunning = false;
//             break;
//         }
//     }
// }

// int main() {
//     std::cout << "=== Samloc Client - SFML UI ===\n\n";

//     // Network config
//     NetworkConfig netCfg;
//     netCfg.serverIp = "127.0.0.1";
//     netCfg.serverPort = 5000;

//     // ===== Input sender =====
//     uint32_t senderId;
//     std::cout << "Enter your userId: ";
//     std::cin >> senderId;
//     std::cin.ignore(); // Xóa bộ đệm sau khi nhập số
//     // Event queue
//     ThreadSafeQueue<NetworkEvent> eventQueue;

//     // Network client (background thread)
//     NetworkClient networkClient(netCfg, eventQueue);
//     if (!networkClient.start()) {
//         std::cerr << "Failed to start network client\n";
//         return 1;
//     }

//     // Session
//     ClientSession session;

//     // Load font
//     sf::Font font;
//     if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")) {
//         std::cerr << "Failed to load font. Using default (may not render).\n";
//         // Continue anyway; SFML will use a default font
//     }

//     // ===== Input receiver =====
//     uint32_t receiverId;
//     std::cout << "Enter receiverId (Person you want to chat with): ";
//     std::cin >> receiverId;
//     std::cin.ignore();

//     std::cout << "\n=== Start chatting (Type 'exit' to quit) ===\n";

//     // ===== BƯỚC 1: KHỞI TẠO LUỒNG LẮNG NGHE (RECEIVER THREAD) =====
//     // Tạo một luồng mới, truyền vào con trỏ của socket và handler
//     std::thread receiverThread(receiveTask, &socket, &connHandler);

//     // ===== BƯỚC 2: VÒNG LẶP NHẬP LIỆU (MAIN THREAD) =====
//     std::string text;
//     while (isRunning) {
//         std::cout << "You: ";
//         std::getline(std::cin, text);

//         if (text == "exit") {
//             isRunning = false;
//             break;
//         }

//         // Gửi tin nhắn
//         if (!text.empty()) {
//             chatHandler.onSendPrivateChat(receiverId, text);
//         }
//     }

//     // ===== Cleanup =====
//     std::cout << "Disconnecting...\n";
    
//     // Ngắt kết nối socket để luồng receiveTask thoát khỏi trạng thái blocking (chờ recv)
//     socket.disconnect(); 

//     // Chờ luồng phụ kết thúc rồi mới thoát chương trình chính
//     if (receiverThread.joinable()) {
//         receiverThread.join();
//     }

//     std::cout << "\nClient closed\n";
//     // SFML window
//     sf::RenderWindow window(sf::VideoMode(1280, 720), "Samloc - Casino Card Game");
//     window.setFramerateLimit(60);

//     // State context
//     StateContext stateCtx(networkClient, session, eventQueue, font);

//     // Game manager
//     GameManager gameManager(stateCtx);

//     // Clock for delta time
//     sf::Clock clock;

//     // Main loop
//     while (window.isOpen()) {
//         sf::Event event;
//         while (window.pollEvent(event)) {
//             if (event.type == sf::Event::Closed) {
//                 window.close();
//             }

//             sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
//             gameManager.handleEvent(event, mousePos);
//         }

//         float dt = clock.restart().asSeconds();
//         gameManager.update(dt);

//         window.clear();
//         gameManager.draw(window);
//         window.display();
//     }

//     networkClient.stop();
//     std::cout << "Client closed\n";
//     return 0;
// }