#include <iostream>
#include <string>

#include "net/client_socket.h"
#include "net/chat/message_sender.h"
#include "logic/chat/chat_logic.h"
#include "net/protocol.h"

int main() {
    std::cout << "=== Samloc Client - Direct Chat Test ===\n\n";

    // ===== Server config =====
    std::string serverIp = "127.0.0.1";
    int serverPort = 5000;

    // ===== User info (test cứng) =====
    uint32_t userId = 1001;          // senderId
    std::string token = "test_token";

    // ===== Create socket =====
    ClientSocket socket(serverIp, serverPort);

    std::cout << "Connecting to server "
              << serverIp << ":" << serverPort << "...\n";

    if (!socket.connect()) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }

    std::cout << "Connected successfully!\n\n";

    // ===== Create sender & logic =====
    MessageSender messageSender(socket, userId, token);
    ChatLogic chatLogic(messageSender);

    uint32_t receiverId;
    std::cout << "Enter receiverId: ";
    std::cin >> receiverId;
    std::cin.ignore(); // bỏ newline

    std::string message;

    // ===== Main loop =====
    while (true) {
        std::cout << "\nEnter message (or 'quit'): ";
        std::getline(std::cin, message);

        if (message == "quit" || message == "exit") {
            break;
        }

        // ===== Send direct chat =====
        if (!chatLogic.sendDirectMessage(receiverId, message)) {
            std::cout << "Failed to send message\n";
            continue;
        }

        std::cout << "Message sent, waiting for ACK...\n";

        // ===== Receive ACK =====
        Message ackMsg = socket.receiveMessage();

        if (ackMsg.header.messageType ==
            static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK)) {

            std::cout << "[SERVER ACK] "
                      << ackMsg.payload << std::endl;
        } else {
            std::cout << "Received unexpected message type: "
                      << ackMsg.header.messageType << std::endl;
        }
    }

    socket.disconnect();
    std::cout << "\nClient closed\n";
    return 0;
}
