#include "chat_handler.h"
#include <iostream>

ChatHandler::ChatHandler(ChatLogic& logic, ClientSession& session):
    chatLogic_(logic),
    session_(session){}

void ChatHandler:: onSendPrivateChat(uint32_t receiverId, const std::string& message){
    //chi gui khi dang o private chat
    if(session_.state() != ClientState::IN_PRIVATE_CHAT){
        std::cerr << "[ChatHandler] Invalid state to send private chat\n";
        return;
    }

    //fsm transition
    session_.setReturnState(session_.state());
    session_.setState(ClientState::SENDING_MESSAGE);

    // call logic
    if (!chatLogic_.sendDirectMessage(receiverId, message)){
        //xu ly loi
        session_.setState(session_.returnState());
        std::cerr << "[ChatHandler] Send private chat failed\n";
    }
}

void ChatHandler::onServerACK(const Message& ackMsg){
    if(ackMsg.header.messageType != static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK)){
        return;
    }

    //Nhan duoc ack --> chuyen ve state cu
    if(session_.state() == ClientState::SENDING_MESSAGE){
        session_.setState(session_.returnState());
    }
    std::cout << "[ChatHandler] Server ACK received\n";
}

void ChatHandler::onServerDeliverMessage(const Message& msg){
    uint32_t senderId = msg.header.senderId;

    // ===== Parse message content =====
    const std::string& message = msg.payload;

    // ===== Route to GUI (console for now) =====
    std::cout << "\n[Private Chat]\n";
    std::cout << "From user: " << senderId << "\n";
    std::cout << "Message  : " << message << "\n";
    std::cout << "--------------------------\n";

    std::cout << "[ChatHandler] New message delivered\n";
}