#include "connection_handler.h"

#include "../chat/chat_handler.h"
#include "../../logic/chat/chat_logic.h"
#include "../session/session_manager.h"

#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(int fd)
    : clientFd(fd), boundUserId(0) {
        inputBuffer.reserve(4096);
    }

int ConnectionHandler::getFd() const {
    return clientFd;
}

bool ConnectionHandler::sendAll(const char* data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t s = send(clientFd, data + sent, size - sent, 0);
        if (s < 0) {
            if (errno == EINTR) continue;
            std::cerr << "[Server] send error fd=" << clientFd
                      << " errno=" << errno
                      << " (" << strerror(errno) << ")\n";
            return false;
        }
        if (s == 0) {
            std::cerr << "[Server] send returned 0 fd=" << clientFd << "\n";
            return false;
        }
        sent += static_cast<size_t>(s);
    }
    return true;
}


bool ConnectionHandler::sendMessage(const Message& msg) {
    std::string bytes = msg.serialize();
    return sendAll(bytes.data(), bytes.size());
}


/**
 * Xử lý 1 event READ
 * return false => client disconnect
 */
bool ConnectionHandler::onReadable() {
    char tempBuf[4096];
    ssize_t bytesRead = recv(clientFd, tempBuf, sizeof(tempBuf), 0);

    if(bytesRead > 0){
        inputBuffer.insert(inputBuffer.end(), tempBuf, tempBuf + bytesRead);
    } else if (bytesRead ==0){
        //client close connection
        return false;
    }else{
        //bytesRead<0: kiem tra loi
        if(errno == EAGAIN || errno ==EWOULDBLOCK){
            //khong co du lieu, giu connection, cho lan poll sau
            return true;
        }

        // Lỗi thật sự
        std::cerr << "[Server] recv error fd=" << clientFd
                  << " errno=" << errno << " (" << strerror(errno) << ")\n";
        return false;

    }

    //Parse message va handle
    while(true){
        //check header
        if (inputBuffer.size() < sizeof(MessageHeader)){
            break; //Chưa đủ header -> thoát, chờ lần đọc sau
        }

        //doc header
        MessageHeader* header = reinterpret_cast<MessageHeader*>(inputBuffer.data());
        
        size_t totalMsgSize = sizeof(MessageHeader) + header->payloadLength;

        //Kiem tra da du payload chua
        if (inputBuffer.size() < totalMsgSize) {
            break; // Đã có header, biết cần bao nhiêu byte nữa, nhưng chưa đủ -> chờ
        }

        //da co du 1 goi tin hoan chinh --> trich xuat
        std::string payload;

        if(header->payloadLength > 0){
            payload.assign(inputBuffer.begin() + sizeof(MessageHeader), inputBuffer.begin() + totalMsgSize); //payload.assign(first, last)-->sao chep du lieu tu first toi last
        }

        Message msg;
        msg.header = *header;
        msg.payload = payload;

        processIncomingMessage(msg);

        //Xóa gói tin đã xử lý khỏi buffer để trượt sang gói tiếp theo
        inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + totalMsgSize);
    }
    return true;
}

void ConnectionHandler::processIncomingMessage(const Message& incoming){
    //bind session lan dau neu chua co
    if(boundUserId == 0 && incoming.header.senderId!=0){
        boundUserId = incoming.header.senderId;
        SessionManager::instance().add(boundUserId, this);
        std::cout<< "[Server] User " << boundUserId << " bound to fd " << clientFd << std::endl;

    }

    ChatLogic chatLogic;
    ChatHandler chatHandler(chatLogic);

    Message response;
    bool needRespond = false;

    switch (static_cast<MessageType>(incoming.header.messageType)) {
        case MessageType::CHAT_DIRECT:
            response = chatHandler.handleChatDirect(incoming);
            needRespond = true;
            break;
        default:
            std::cerr << "[Server] Unsupported message type: " 
                      << incoming.header.messageType << std::endl;
            return;
    }

    if (needRespond) {
        std::cout << "[Server] Sending response type=" << response.header.messageType 
                  << " to fd=" << clientFd << std::endl;
        
        if (!sendMessage(response)) {
            std::cerr << "[Server] Failed to send response to fd=" << clientFd << std::endl;
        }
    }
}

void ConnectionHandler::closeConnection() {
    if (boundUserId != 0) {
        SessionManager::instance().remove(boundUserId);
        std::cout << "[Server] User " << boundUserId << " unbound\n";
    }
    close(clientFd);
}
