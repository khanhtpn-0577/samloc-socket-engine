#pragma once

#include <string>
#include <cstdint>

/**
 * ClientState
 *  - FSM trạng thái phía client
 */

 enum class ClientState {
    LOGGED_OUT,
    LOGGING_IN,
    LOGGED_IN,
    DISCONNECTED,
    ERROR
 };

 enum class ChatState {
    CHAT_IDLE,
    CHAT_WAIT_ACK,
    CHAT_FAILED
};


/**
 * ClientSession
 *  - Quản lý FSM phía client và auth state
 */

class ClientSession{
    public:
        ClientSession();

        // get current state
        ClientState state() const;

        //set state
        void setState(ClientState newState);

        // Auth state
        bool isLoggedIn() const;
        void setLoggedIn(bool loggedIn);
        
        uint32_t userId() const;
        void setUserId(uint32_t id);
        
        std::string token() const;
        void setToken(const std::string& tok);
        
        std::string username() const;
        void setUsername(const std::string& name);
        
        ChatState chatState() const;

        void setChatState(ChatState state);

        double balance() const;
        void setBalance(double bal);

        bool canSendChat() const; 
    
    private:
        ClientState currentState_;
        
        // Auth state
        bool loggedIn_;
        uint32_t userId_;
        std::string token_;
        std::string username_;
        double balance_;

        ChatState chatState_;
};