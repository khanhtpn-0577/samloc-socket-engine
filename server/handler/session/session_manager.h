#pragma once
#include <unordered_map>
#include <mutex>

class ConnectionHandler;

class SessionManager {
public:
    static SessionManager& instance();

    void add(uint32_t userId, ConnectionHandler* handler);
    void remove(uint32_t userId);

    ConnectionHandler* get(uint32_t userId);

private:
    SessionManager() = default;

    std::unordered_map<uint32_t, ConnectionHandler*> sessions;
    std::mutex mtx;
};
