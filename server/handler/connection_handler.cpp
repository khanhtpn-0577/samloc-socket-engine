#include "connection_handler.h"
#include "../net/protocol.h"
#include "../logic/chat/chat_logic.h"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <chrono>


ConnectionHandler::ConnectionHandler(int clientFd) //constructor
    : clientFd(clientFd){} // tuong duong this.clientFd = clientFd

bool ConnectionHandler::recvAll(void* buffer, size_t size){
    size_t received = 0; //size_t la kieu du lieu unsigned, du lon de chua kich thuoc toi da cua bat ky object nao
    char* buf = static_cast<char*>(buffer); //cast buffer tu kieu void sang kieu char
    
    while (received < size){
        ssize_t r = recv(clientFd, buf + received, size - received, 0); // cung do lon voi size_t nhung la signed, tuc la bieu dien duoc so am // buf + received: vi tri ghi du lieu
        if (r<=0) return false;
        received += r;
    }
    return true;

}

bool ConnectionHandler::sendAll(const void* buffer, size_t size){
    size_t sent = 0;
    const char* buf = static_cast<const char*>(buffer);

    while(sent < size){
        ssize_t s = send(clientFd, buf + sent, size - sent, 0);
        if (s<=0) return false;
        sent += s;
    }
    return true;
}

void ConnectionHandler::handle(){
    ChatLogic chatLogic;

    MessageHeader header;
    if (!recvAll(&header, sizeof(header))){
        close(clientFd);
        return;
    }

    std::string payload;
    if(header.payloadLength > 0){
        payload.resize(header.payloadLength); //cap phat bo nho cho payload theo payloadLenth
        if (!recvAll(payload.data(), header.payloadLength)){
            close(clientFd);
            return;
        }
    }

    Message incomingMsg;
    incomingMsg.header = header;
    incomingMsg.payload = payload;

    MessageType type = static_cast<MessageType>(incomingMsg.header.messageType);

    if (type == MessageType::CHAT_DIRECT){
        // payload = receivedId(4byte) + message text
        uint32_t receiverId;
        std::memcpy(&receiverId, incomingMsg.payload.data(), sizeof(uint32_t));
        receiverId = ntohl(receiverId); // chuyen tu big edian thanh littel edian
        
        std::string message = incomingMsg.payload.substr(sizeof(uint32_t)); //substr: lay chuoi con tu vi tri sizeof(uint32_t) den het
        
        //Call logic
        std::string ackText = 
            chatLogic.handleDirectChat(
                header.senderId,
                receiverId,
                message
            );
        
        //Build ACK message
        Message ackMsg;
        ackMsg.header.messageType = static_cast<uint16_t>(MessageType::CHAT_DIRECT_ACK);
        ackMsg.header.senderId = 0;
        ackMsg.header.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        ackMsg.header.payloadLength = ackText.size();
        std::memset(ackMsg.header.token, 0, 32);

        ackMsg.payload = ackText;

        // Serialize & send
        std::string bytes = ackMsg.serialize();
        sendAll(bytes.data(), bytes.size());
    }
    close(clientFd);
}


