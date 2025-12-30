#include "client_session.h"

//constructor
ClientSession::ClientSession():
    currentState_(ClientState::LOGGED_OUT),
    returnState_(ClientState::LOGGED_OUT),
    loggedIn_(false),
    userId_(0),
    token_(""),
    username_(""){}

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

// Auth state
bool ClientSession::isLoggedIn() const {
    return loggedIn_;
}

void ClientSession::setLoggedIn(bool loggedIn) {
    loggedIn_ = loggedIn;
}

uint32_t ClientSession::userId() const {
    return userId_;
}

void ClientSession::setUserId(uint32_t id) {
    userId_ = id;
}

std::string ClientSession::token() const {
    return token_;
}

void ClientSession::setToken(const std::string& tok) {
    token_ = tok;
}

std::string ClientSession::username() const {
    return username_;
}

void ClientSession::setUsername(const std::string& name) {
    username_ = name;
}