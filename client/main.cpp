#include <iostream>
#include <string>

#include "net/client_socket.h"
#include "net/chat/message_sender.h"
#include "logic/chat/chat_logic.h"
#include "net/protocol.h"
#include "handlers/chat/chat_handler.h"

int main() {
    std::cout << "=== Samloc Client - Direct Chat Test ===\n\n";

    // ===== Server config =====
    std::string serverIp = "127.0.0.1";
    int serverPort = 5000;

    // ===== User info (test cứng) =====
    uint32_t userId = 1002;
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

    // ===== Create core components =====
    MessageSender messageSender(socket, userId, token);
    ChatLogic chatLogic(messageSender);

    ClientSession session;
    ChatHandler chatHandler(chatLogic, session);

    // ===== Enter private chat =====
    session.setState(ClientState::IN_PRIVATE_CHAT);

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

        // ===== GUI → Handler =====
        chatHandler.onSendPrivateChat(receiverId, message);

        // ===== Blocking receive (demo đơn giản) =====
        Message serverMsg = socket.receiveMessage();

        if (serverMsg.header.messageType ==
            static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK)) {

            chatHandler.onServerACK(serverMsg);
            std::cout << "[SERVER ACK] "
                      << serverMsg.payload << "\n";
        } else {
            // sau này có thể là deliver message
            chatHandler.onServerDeliverMessage(serverMsg);
        }
    }

    socket.disconnect();
    std::cout << "\nClient closed\n";
    return 0;
}
