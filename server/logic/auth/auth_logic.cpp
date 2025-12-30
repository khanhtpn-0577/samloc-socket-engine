#include "auth_logic.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <sqlite3.h>

AuthLogic::AuthLogic(Database& db) : db(db) {}

int64_t AuthLogic::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

std::string AuthLogic::generateSalt() const {
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

std::string AuthLogic::generateToken() const {
    unsigned char token[32];
    RAND_bytes(token, sizeof(token));
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)token[i];
    }
    return ss.str();
}

std::string AuthLogic::hashPassword(const std::string& password, const std::string& salt) const {
    std::string combined = password + salt;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

SignupResult AuthLogic::signup(
    const std::string& username,
    const std::string& password,
    const std::string& displayName
) {
    SignupResult result{false, "", 0};
    
    // Validate input
    if (username.empty() || username.length() < 3) {
        result.message = "Username must be at least 3 characters";
        return result;
    }
    
    if (password.empty() || password.length() < 6) {
        result.message = "Password must be at least 6 characters";
        return result;
    }
    
    // Check if username already exists
    QueryResult checkResult = db.queryPrepared(
        "SELECT player_id FROM players WHERE username = ?;",
        {username}
    );
    
    if (!checkResult.empty()) {
        result.message = "Username already exists";
        return result;
    }
    
    // Generate salt and hash password
    std::string salt = generateSalt();
    std::string passwordHash = hashPassword(password, salt);
    std::string storedHash = salt + ":" + passwordHash;
    
    // Insert user (set legacy plaintext `password` to empty string to satisfy NOT NULL)
    bool inserted = db.executePrepared(
        "INSERT INTO players (username, password, password_hash, display_name, balance, status) "
        "VALUES (?, '', ?, ?, 0, 'online');",
        {username, storedHash, displayName.empty() ? username : displayName}
    );
    
    if (!inserted) {
        result.message = "Database error during signup";
        return result;
    }
    
    // Get the inserted user ID
    int64_t userId = db.getLastInsertId();
    
    result.success = true;
    result.userId = static_cast<uint32_t>(userId);
    result.message = "Signup successful";
    
    std::cout << "[AuthLogic] User registered: " << username
              << " (id=" << result.userId << ")\n";
    
    return result;
}

LoginResult AuthLogic::login(
    const std::string& username,
    const std::string& password
) {
    LoginResult result{false, "", 0, ""};
    
    if (username.empty() || password.empty()) {
        result.message = "Username and password required";
        return result;
    }
    
    // Get user from database
    QueryResult userResult = db.queryPrepared(
        "SELECT player_id, password_hash FROM players WHERE username = ?;",
        {username}
    );
    
    if (userResult.empty()) {
        result.message = "Invalid username or password";
        return result;
    }
    
    // Parse stored hash (salt:hash)
    std::string storedHash = userResult[0]["password_hash"];
    size_t colonPos = storedHash.find(':');
    
    if (colonPos == std::string::npos) {
        result.message = "Invalid password format in database";
        return result;
    }
    
    std::string salt = storedHash.substr(0, colonPos);
    std::string expectedHash = storedHash.substr(colonPos + 1);
    
    // Verify password
    std::string actualHash = hashPassword(password, salt);
    
    if (actualHash != expectedHash) {
        result.message = "Invalid username or password";
        return result;
    }
    
    // Get user ID
    uint32_t userId = std::stoul(userResult[0]["player_id"]);
    
    // Generate session token
    std::string token = generateToken();
    int64_t expiresAt = getCurrentTimestamp() + SESSION_TTL_MS;
    
    // Create session
    bool sessionCreated = db.executePrepared(
        "INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, ?);",
        {std::to_string(userId), token, std::to_string(expiresAt)}
    );
    
    if (!sessionCreated) {
        result.message = "Failed to create session";
        return result;
    }
    
    result.success = true;
    result.userId = userId;
    result.token = token;
    result.message = "Login successful";
    
    std::cout << "[AuthLogic] User logged in: " << username
              << " (id=" << userId << ", token=" << token.substr(0, 8) << "...)\n";
    
    return result;
}

bool AuthLogic::logout(const std::string& token) {
    if (token.empty()) {
        return false;
    }
    
    bool deleted = db.executePrepared(
        "DELETE FROM sessions WHERE token = ?;",
        {token}
    );
    
    if (deleted) {
        std::cout << "[AuthLogic] Session logged out: " << token.substr(0, 8) << "...\n";
    }
    
    return deleted;
}

std::optional<SessionInfo> AuthLogic::verifySession(const std::string& token) {
    if (token.empty()) {
        return std::nullopt;
    }
    
    int64_t now = getCurrentTimestamp();
    
    QueryResult sessionResult = db.queryPrepared(
        "SELECT user_id, token, expires_at FROM sessions WHERE token = ? AND expires_at > ?;",
        {token, std::to_string(now)}
    );
    
    if (sessionResult.empty()) {
        std::cout << "[AuthLogic] Invalid or expired session: " << token.substr(0, 8) << "...\n";
        return std::nullopt;
    }
    
    SessionInfo info;
    info.userId = std::stoul(sessionResult[0]["user_id"]);
    info.token = sessionResult[0]["token"];
    info.expiresAt = std::stoll(sessionResult[0]["expires_at"]);
    
    std::cout << "[AuthLogic] Session verified: userId=" << info.userId
              << ", token=" << token.substr(0, 8) << "...\n";
    
    return info;
}

void AuthLogic::cleanExpiredSessions() {
    int64_t now = getCurrentTimestamp();
    
    bool cleaned = db.executePrepared(
        "DELETE FROM sessions WHERE expires_at <= ?;",
        {std::to_string(now)}
    );
    
    if (cleaned) {
        std::cout << "[AuthLogic] Cleaned expired sessions\n";
    }
}
