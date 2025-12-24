#include "client_connection_handler.h"
#include "../chat/chat_handler.h"
#include <iostream>

//constructor
ClientConnectionHandler::ClientConnectionHandler(
    ChatHandler& chatHandler
):
    chatHandler_(chatHandler){}

void ClientConnectionHandler::handleMessage(const Message& message){
    MessageType type = static_cast<MessageType>(message.header.messageType);
    std::cout << "[ClientConnectionHandler] Received message. "
              << "type=0x"
              << std::hex << message.header.messageType
              << std::dec << "\n";

    switch (type) {
        case MessageType::CHAT_DIRECT_ACK:
            chatHandler_.onServerACK(message);
            break;

        case MessageType::CHAT_DIRECT:
            chatHandler_.onServerDeliverMessage(message);
            break;

        default:
            std::cerr << "[ClientConnectionHandler] "
                      << "Unhandled message type: "
                      << message.header.messageType << "\n";
            break;
    }
}
