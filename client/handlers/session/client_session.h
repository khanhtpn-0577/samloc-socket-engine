#pragma once

/**
 * ClientState
 *  - FSM trạng thái phía client
 */

 enum class ClientState {
    IN_ROOM,
    IN_PRIVATE_CHAT,
    SENDING_MESSAGE,
    DISCONNECTED,
    ERROR
 };

/**
 * ClientSession
 *  - Quản lý FSM phía client
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
    
    private:
        ClientState currentState_;
        ClientState returnState_;
};