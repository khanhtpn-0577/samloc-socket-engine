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

void ChatHandler::onServerDeliverMessage(const Message& msg) {
    uint32_t senderId = msg.header.senderId;
    std::string content = chatLogic_.normalizeChatContent(msg.payload);

    if (incomingMessageCallback_) {
        incomingMessageCallback_(senderId, content);
    }
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

void ChatHandler::requestFriendList(){
    std::cout << "[ChatHandler] Requesting friend list from server...\n";
    if (!session_.isLoggedIn()) return;
    chatLogic_.requestFriendList(session_.userId());
}

void ChatHandler::onServerDeliverFriendList(const Message& message) {
    const std::string& payload = message.payload;

    std::vector<FriendInfo> friends;

    size_t pos = payload.find("\"friends\"");
    if (pos == std::string::npos) {
        std::cerr << "[ChatHandler] Invalid friend list payload\n";
        return;
    }

    pos = payload.find("[", pos);
    if (pos == std::string::npos) return;
    ++pos; // move past '['

    while (true) {
        size_t idPos = payload.find("\"id\":", pos);
        if (idPos == std::string::npos) break;

        size_t idStart = idPos + 5;
        size_t idEnd = payload.find(",", idStart);
        uint32_t id =
            static_cast<uint32_t>(
                std::stoul(payload.substr(idStart, idEnd - idStart))
            );

        size_t namePos = payload.find("\"name\":\"", idEnd);
        if (namePos == std::string::npos) break;

        size_t nameStart = namePos + 8;
        size_t nameEnd = payload.find("\"", nameStart);
        std::string name =
            payload.substr(nameStart, nameEnd - nameStart);

        friends.push_back({id, name});

        pos = nameEnd;
    }

    std::cout << "[ChatHandler] Received friend list: "
              << friends.size() << " friends\n";

    // Notify UI
    if (friendListCallback_) {
        friendListCallback_(friends);
    }
}

void ChatHandler::setFriendListCallback(FriendListCallback callback) {
    friendListCallback_ = callback;
}

void ChatHandler::requestPrivateChatHistory(uint32_t peerId) {
    chatLogic_.requestPrivateChatHistory(peerId);
}

void ChatHandler::onServerDeliverPrivateChatHistory(
    const Message& message
) {
    const std::string& payload = message.payload;

    std::vector<ChatHistoryItem> history;

    // Tìm "messages"
    size_t pos = payload.find("\"messages\"");
    if (pos == std::string::npos) {
        std::cerr << "[ChatHandler] Invalid history payload\n";
        return;
    }

    pos = payload.find("[", pos);
    if (pos == std::string::npos) return;
    ++pos; // skip '['

    while (true) {
        size_t senderPos = payload.find("\"senderId\":", pos);
        if (senderPos == std::string::npos) break;

        size_t senderStart = senderPos + 11;
        size_t senderEnd = payload.find(",", senderStart);
        uint32_t senderId =
            static_cast<uint32_t>(
                std::stoul(payload.substr(senderStart, senderEnd - senderStart))
            );

        size_t contentPos = payload.find("\"content\":\"", senderEnd);
        if (contentPos == std::string::npos) break;

        size_t contentStart = contentPos + 11;
        size_t contentEnd = payload.find("\"", contentStart);
        std::string content =
            payload.substr(contentStart, contentEnd - contentStart);

        size_t timePos = payload.find("\"sentAt\":\"", contentEnd);
        if (timePos == std::string::npos) break;

        size_t timeStart = timePos + 10;
        size_t timeEnd = payload.find("\"", timeStart);
        std::string sentAt =
            payload.substr(timeStart, timeEnd - timeStart);

        history.push_back({
            senderId,
            content,
            sentAt
        });

        pos = timeEnd;
    }

    std::cout << "[ChatHandler] Received "
              << history.size()
              << " chat history messages\n";

    // Notify UI
    if (chatHistoryCallback_) {
        chatHistoryCallback_(history);
    }
}

void ChatHandler::setChatHistoryCallback(ChatHistoryCallback cb) {
    chatHistoryCallback_ = cb;
}

void ChatHandler::setIncomingMessageCallback(IncomingMessageCallback cb) {
    incomingMessageCallback_ = cb;
}






