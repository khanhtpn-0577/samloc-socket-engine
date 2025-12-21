#include "client_session.h"

//constructor
ClientSession::ClientSession():
    currentState_(ClientState::IN_ROOM),
    returnState_(ClientState::IN_ROOM){}

//getter
ClientState ClientSession::state() const{
    return currentState_;
}

//setter
void ClientSession::setState(ClientState newState){
    currentState_ = newState;
}

void ClientSession::setReturnState(ClientState state){
    returnState_ = state;
}

ClientState ClientSession::returnState() const{
    return returnState_ ;
}

bool ClientSession::isSending() const{
    return currentState_ == ClientState::SENDING_MESSAGE;
}