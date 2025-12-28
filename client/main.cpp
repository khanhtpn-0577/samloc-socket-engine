#include <iostream>
#include <string>
#include <thread>   // Thư viện cho luồng
#include <atomic>   // Thư viện cho biến cờ hiệu an toàn giữa các luồng

#include "net/client_socket.h"
#include "net/chat/message_sender.h"
#include "logic/chat/chat_logic.h"
#include "handlers/chat/chat_handler.h"
#include "handlers/connection/client_connection_handler.h"
#include "net/protocol.h"

// Biến cờ hiệu để kiểm soát vòng lặp, atomic đảm bảo an toàn thread
std::atomic<bool> isRunning(true);

// Hàm lắng nghe tin nhắn (sẽ chạy ở luồng riêng)
void receiveTask(ClientSocket* socket, ClientConnectionHandler* connHandler) {
    while (isRunning) {
        try {
            // Giả sử receiveMessage là blocking (chờ đến khi có tin mới)
            // Cần xử lý trường hợp socket bị đóng hoặc lỗi để break vòng lặp
            Message serverMsg = socket->receiveMessage();
            
            // In dấu xuống dòng để không bị đè lên dòng "You: " đang nhập dở
            std::cout << "\n"; 
            connHandler->handleMessage(serverMsg);
            
            // In lại prompt nhập liệu cho đẹp (optional)
            std::cout << "\nYou: " << std::flush; 
        } catch (const std::exception& e) {
            // Nếu socket lỗi hoặc ngắt kết nối
            if (isRunning) std::cerr << "\nConnection lost: " << e.what() << "\n";
            isRunning = false;
            break;
        }
    }
}

int main() {
    std::cout << "=== Samloc Client - Direct Chat Demo ===\n\n";

    // ===== Server config =====
    std::string serverIp = "127.0.0.1";
    int serverPort = 5000;

    // ===== Input sender =====
    uint32_t senderId;
    std::cout << "Enter your userId: ";
    std::cin >> senderId;
    std::cin.ignore(); // Xóa bộ đệm sau khi nhập số

    std::string token = "test_token";

    // ===== Connect socket =====
    ClientSocket socket(serverIp, serverPort);
    if (!socket.connect()) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }

    std::cout << "Connected to server\n";

    // ===== Core components =====
    MessageSender messageSender(socket, senderId, token);
    ChatLogic chatLogic(messageSender);

    ClientSession session;
    session.setState(ClientState::IN_PRIVATE_CHAT);

    ChatHandler chatHandler(chatLogic, session);
    ClientConnectionHandler connHandler(chatHandler);

    // ===== Input receiver =====
    uint32_t receiverId;
    std::cout << "Enter receiverId (Person you want to chat with): ";
    std::cin >> receiverId;
    std::cin.ignore();

    std::cout << "\n=== Start chatting (Type 'exit' to quit) ===\n";

    // ===== BƯỚC 1: KHỞI TẠO LUỒNG LẮNG NGHE (RECEIVER THREAD) =====
    // Tạo một luồng mới, truyền vào con trỏ của socket và handler
    std::thread receiverThread(receiveTask, &socket, &connHandler);

    // ===== BƯỚC 2: VÒNG LẶP NHẬP LIỆU (MAIN THREAD) =====
    std::string text;
    while (isRunning) {
        std::cout << "You: ";
        std::getline(std::cin, text);

        if (text == "exit") {
            isRunning = false;
            break;
        }

        // Gửi tin nhắn
        if (!text.empty()) {
            chatHandler.onSendPrivateChat(receiverId, text);
        }
    }

    // ===== Cleanup =====
    std::cout << "Disconnecting...\n";
    
    // Ngắt kết nối socket để luồng receiveTask thoát khỏi trạng thái blocking (chờ recv)
    socket.disconnect(); 

    // Chờ luồng phụ kết thúc rồi mới thoát chương trình chính
    if (receiverThread.joinable()) {
        receiverThread.join();
    }

    std::cout << "\nClient closed\n";
    return 0;
}