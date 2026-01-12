#pragma once

#include <string>
#include <cstdint>
#include <functional>

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
        using FriendListCallback =
            std::function<void(const std::vector<FriendInfo>&)>;

        ChatHandler(ChatLogic& logic, ClientSession& session);

        void onSendPrivateChat(uint32_t receiverId, const std::string& message);


        void onServerACK(const Message& ackMsg);

        void onServerDeliverMessage(const Message& msg);

        void requestFriendList();

        void onServerDeliverFriendList(const Message& message);

        void setFriendListCallback(FriendListCallback cb);

        void requestPrivateChatHistory(uint32_t friendId);

        void onServerDeliverPrivateChatHistory(const Message& message);

        using ChatHistoryCallback = std::function<void(const std::vector<ChatHistoryItem>&)>;

        void setChatHistoryCallback(ChatHistoryCallback cb);

        using IncomingMessageCallback = std::function<void(uint32_t senderId, const std::string& message)>;

        void setIncomingMessageCallback(IncomingMessageCallback cb);

        void onSendRoomChat(int roomId, const std::string& message);




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
        FriendListCallback friendListCallback_;
        ChatHistoryCallback chatHistoryCallback_;
        IncomingMessageCallback incomingMessageCallback_;

};
