#include "chat_handler.h"
#include <iostream>

ChatHandler::ChatHandler(ChatLogic& logic, ClientSession& session):
    chatLogic_(logic),
    session_(session){}

void ChatHandler:: onSendPrivateChat(uint32_t receiverId, const std::string& message){
    
    // ===== FSM guard =====
    if (session_.chatState() != ChatState::CHAT_IDLE) {
        std::cerr << "[ChatHandler] Chat is busy, cannot send\n";
        return;
    }

    // call logic
    if (!chatLogic_.sendDirectMessage(receiverId, message)){
        //xu ly loi
        std::cerr << "[ChatHandler] Send private chat failed\n";
        session_.setChatState(ChatState::CHAT_FAILED);
        return;
    }

    // ===== FSM transition =====
    session_.setChatState(ChatState::CHAT_WAIT_ACK);
    startAckTimer();
    std::cout << "[ChatHandler] Message sent, waiting for ACK\n";

}

void ChatHandler::onServerACK(const Message& ackMsg){
    if(ackMsg.header.messageType != static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK)){
        return;
    }

    if(session_.chatState() != ChatState::CHAT_WAIT_ACK){
        //ACK tre/duplicate --> ignore
        std::cerr << "[ChatHandler] Unexpected ACK received, ignoring\n";
        return;
    }

    // Stop ACK timer
    stopAckTimer();

    session_.setChatState(ChatState::CHAT_IDLE);

    std::cout << "[ChatHandler] Server ACK received, chat idle\n";

    // TODO: update UI → sent
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

void ChatHandler::startAckTimer(){
    stopAckTimer();

    ackTimerActive_ = true;

    ackTimerThread_ = std::thread([this](){
        constexpr int ACK_TIMEOUT_SEC = 10;

        //chia 10s thanh 100 lan 100ms de kiem tra ackTimerActive_
        for(int i = 0; i < ACK_TIMEOUT_SEC * 10; ++i){
            if(!ackTimerActive_){
                return; //ack da den
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        //timeout
        if(ackTimerActive_){
            onAckTimeout();
        }
    });
    ackTimerThread_.detach(); //tach luong de no chay doc lap
}

void ChatHandler::stopAckTimer(){
    ackTimerActive_ = false;
}

void ChatHandler::onAckTimeout(){
    std::lock_guard<std::mutex> lock(ackTimerMutex_);

    if(session_.chatState() != ChatState::CHAT_WAIT_ACK){
        return; //da nhan dc ack
    }

    std::cerr << "[ChatHandler] ACK timeout, chat failed\n";
    session_.setChatState(ChatState::CHAT_FAILED);
    //TODO: notify UI ve loi
}