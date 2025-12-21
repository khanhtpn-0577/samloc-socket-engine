#include "session_manager.h"
#include "../connection/connection_handler.h"

SessionManager& SessionManager::instance() {
    static SessionManager inst;
    return inst;
}

void SessionManager::add(uint32_t userId, ConnectionHandler* handler) {
    std::lock_guard<std::mutex> lock(mtx);
    sessions[userId] = handler;
}

void SessionManager::remove(uint32_t userId) {
    std::lock_guard<std::mutex> lock(mtx);
    sessions.erase(userId);
}

ConnectionHandler* SessionManager::get(uint32_t userId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = sessions.find(userId);
    return (it == sessions.end()) ? nullptr : it->second;
}
