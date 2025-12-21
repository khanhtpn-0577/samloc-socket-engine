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

    switch (type) {
        case MessageType::CHAT_DIRECT_ACK:
            chatHandler_.onServerACK(message);
            break;

        case MessageType::SERVER_DELIVER_MESSAGE:
            chatHandler_.onServerDeliverMessage(message);
            break;

        default:
            std::cerr << "[ClientConnectionHandler] "
                      << "Unhandled message type: "
                      << message.header.messageType << "\n";
            break;
    }
}
