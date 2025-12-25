// #include <iostream>
// #include <string>

// #include "net/client_socket.h"
// #include "net/chat/message_sender.h"
// #include "logic/chat/chat_logic.h"
// #include "handlers/chat/chat_handler.h"
// #include "handlers/connection/client_connection_handler.h"
// #include "net/protocol.h"

// int main() {
//     std::cout << "=== Samloc Client - Direct Chat Demo ===\n\n";

//     // ===== Server config =====
//     std::string serverIp = "127.0.0.1";
//     int serverPort = 5000;

//     // ===== Input sender =====
//     uint32_t senderId;
//     std::cout << "Enter your userId: ";
//     std::cin >> senderId;
//     std::cin.ignore();

//     std::string token = "test_token";

//     // ===== Connect socket =====
//     ClientSocket socket(serverIp, serverPort);
//     if (!socket.connect()) {
//         std::cerr << "Failed to connect to server\n";
//         return 1;
//     }

//     std::cout << "Connected to server\n";

//     // ===== Core components =====
//     MessageSender messageSender(socket, senderId, token);
//     ChatLogic chatLogic(messageSender);

//     ClientSession session;
//     session.setState(ClientState::IN_PRIVATE_CHAT);

//     ChatHandler chatHandler(chatLogic, session);
//     ClientConnectionHandler connHandler(chatHandler);

//     // ===== Input receiver =====
//     uint32_t receiverId;
//     std::cout << "Enter receiverId: ";
//     std::cin >> receiverId;
//     std::cin.ignore();

//     std::cout << "\n=== Start chatting ===\n";

//     std::string text;
//     std::cout << "\nYou: ";
//     std::getline(std::cin, text);

//     chatHandler.onSendPrivateChat(receiverId, text);

//     // ===== Main loop =====
    
//     while (true) {
    
//         Message serverMsg = socket.receiveMessage();
//         connHandler.handleMessage(serverMsg);
//     }   

//     socket.disconnect();
//     std::cout << "\nClient closed\n";
//     return 0;
// }

#include <SFML/Graphics.hpp>
#include "gui/include/ChatScreen.h"

int main() {
    // Create a SFML window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Samloc Client - Chat");

    // Create and run the ChatScreen
    ChatScreen chatScreen;
    chatScreen.run(window);

    return 0;
}
