#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include "../../db/database.h"

struct ChallengeResult {
    bool success;
    std::string message;
    uint32_t challengeId;
};

struct ChallengeInfo {
    uint32_t challengeId;
    uint32_t senderId;
    uint32_t receiverId;
    std::string status;  // pending | accepted | rejected | cancelled | expired
    int64_t createdAt;
    int64_t expiresAt;
};

/**
 * ChallengeLogic
 *  - Handles challenge lifecycle (send, accept, reject, cancel, expire)
 *  - Based on FSM design in doc/challenge and doc/challenge-response
 *  - Sender FSM: NoChallenge → Sent → (Expired|Cancelled|MatchedIfAuto)
 *  - Receiver FSM: Received → (Accepted|Rejected|Expired) → CreatingRoom → InRoom
 */
class ChallengeLogic {
public:
    explicit ChallengeLogic(Database& db);

    // Send challenge (sender side)
    ChallengeResult sendChallenge(uint32_t senderId, uint32_t receiverId);

    // Cancel challenge (sender side)
    ChallengeResult cancelChallenge(uint32_t challengeId, uint32_t senderId);

    // Accept challenge (receiver side)
    ChallengeResult acceptChallenge(uint32_t challengeId, uint32_t receiverId);

    // Reject challenge (receiver side)
    ChallengeResult rejectChallenge(uint32_t challengeId, uint32_t receiverId);

    // Expire challenges (background worker)
    void expireOldChallenges();

    // Get active challenge for user
    std::optional<ChallengeInfo> getActiveChallenge(uint32_t userId);

    // Get pending challenge sent to user
    std::optional<ChallengeInfo> getPendingChallengeToUser(uint32_t userId);

private:
    int64_t getCurrentTimestamp() const;

private:
    Database& db;
    static constexpr int64_t CHALLENGE_TTL_MS = 30 * 1000; // 30 seconds
};
