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
    IN_ROOM,
    IN_PRIVATE_CHAT,
    SENDING_MESSAGE,
    DISCONNECTED,
    ERROR
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

        // return state(dung khi sending message de biet can quay ve state nao sau khi gui tin nhan xong)
        void setReturnState(ClientState state);

        //getter return state
        ClientState returnState() const;

        bool isSending() const;

        // Auth state
        bool isLoggedIn() const;
        void setLoggedIn(bool loggedIn);
        
        uint32_t userId() const;
        void setUserId(uint32_t id);
        
        std::string token() const;
        void setToken(const std::string& tok);
        
        std::string username() const;
        void setUsername(const std::string& name);
    
    private:
        ClientState currentState_;
        ClientState returnState_;
        
        // Auth state
        bool loggedIn_;
        uint32_t userId_;
        std::string token_;
        std::string username_;
};