#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include "../../db/database.h"

struct SignupResult {
    bool success;
    std::string message;
    uint32_t userId;  // only valid if success=true
};

struct LoginResult {
    bool success;
    std::string message;
    uint32_t userId;
    std::string token;
};

struct SessionInfo {
    uint32_t userId;
    std::string token;
    int64_t expiresAt;
};

/**
 * AuthLogic
 *  - Handles signup, login, logout, session verification
 *  - Uses SQLite for persistence
 *  - Hashes passwords with SHA-256 + salt
 */
class AuthLogic {
public:
    explicit AuthLogic(Database& db);

    // Signup new user
    SignupResult signup(
        const std::string& username,
        const std::string& password,
        const std::string& displayName
    );

    // Login user
    LoginResult login(
        const std::string& username,
        const std::string& password
    );

    // Logout user
    bool logout(const std::string& token);

    // Verify session token
    std::optional<SessionInfo> verifySession(const std::string& token);

    // Clean expired sessions
    void cleanExpiredSessions();

private:
    // Hash password with SHA-256
    std::string hashPassword(const std::string& password, const std::string& salt) const;

    // Generate random salt
    std::string generateSalt() const;

    // Generate random session token
    std::string generateToken() const;

    // Get current timestamp in milliseconds
    int64_t getCurrentTimestamp() const;

private:
    Database& db;
    static constexpr int64_t SESSION_TTL_MS = 24 * 60 * 60 * 1000; // 24 hours
};
