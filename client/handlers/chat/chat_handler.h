#pragma once

#include <string>
#include <cstdint>

#include "../../logic/chat/chat_logic.h"
#include "../session/client_session.h"
#include "../../net/protocol.h"

#include <thread>
#include <atomic>
#include <mutex>


/**
 * ChatHandler
 *  - Entry point cho các use case chat
 *  - Được GUI gọi
 *  - Quan ly FSM
 */

class ChatLogic;
class ClientSession;

class ChatHandler {
    public:
        ChatHandler(ChatLogic& logic, ClientSession& session);

        void onSendPrivateChat(uint32_t receiverId, const std::string& message);


        void onServerACK(const Message& ackMsg);

        void onServerDeliverMessage(const Message& msg);

    private:
        void startAckTimer();
        void stopAckTimer();
        void onAckTimeout();

    private:
        ChatLogic& chatLogic_;
        ClientSession& session_;

        std::thread ackTimerThread_; //tao thread rieng de quan ly timer
        std::atomic<bool> ackTimerActive_{false}; //bien de kiem soat timer, khong dung bool truyen thong vi no khong an toan trong da luong, cu the khi mot luong doc va mot luong ghi
        std::mutex ackTimerMutex_; //mutex de dong bo hoa truy cap den ackTimerActive_
};
