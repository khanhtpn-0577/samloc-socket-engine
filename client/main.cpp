#include <iostream>
#include <string>
#include "network/client_socket.h"

int main() {
    std::cout << "=== Samloc Client - Simple Socket Test ===" << std::endl;
    std::cout << std::endl;
    
    // Server configuration
    std::string serverIp = "127.0.0.1";
    int serverPort = 5000;
    
    // Create client socket
    ClientSocket socket(serverIp, serverPort);
    
    // Connect to server
    std::cout << "Connecting to server " << serverIp << ":" << serverPort << "..." << std::endl;
    if (!socket.connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully connected to server!" << std::endl;
    std::cout << std::endl;
    
    std::string input;
    
    // Main loop - send and receive raw strings
    while (true) {
        std::cout << "Enter message to send (or 'quit' to exit): ";
        std::getline(std::cin, input);
        
        // Check for exit command
        if (input == "quit" || input == "exit") {
            std::cout << "Exiting..." << std::endl;
            break;
        }
        
        // Send raw string to server
        if (!input.empty()) {
            std::cout << "Sending: " << input << std::endl;
            if (socket.send(input)) {
                std::cout << "Message sent successfully!" << std::endl;
                
                // Receive response from server
                std::cout << "Waiting for response..." << std::endl;
                std::string response = socket.receive();
                
                if (!response.empty()) {
                    std::cout << "Response received: " << response << std::endl;
                } else {
                    std::cout << "No response from server" << std::endl;
                }
            } else {
                std::cout << "Failed to send message" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    // Disconnect from server
    socket.disconnect();
    std::cout << "Client closed" << std::endl;
    
    return 0;
}

