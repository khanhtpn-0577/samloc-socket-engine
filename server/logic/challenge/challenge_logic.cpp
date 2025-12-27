#include "challenge_logic.h"
#include <iostream>
#include <chrono>
#include <optional>

ChallengeLogic::ChallengeLogic(Database& db) : db(db) {}

int64_t ChallengeLogic::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

ChallengeResult ChallengeLogic::sendChallenge(uint32_t senderId, uint32_t receiverId) {
    ChallengeResult result{false, "", 0};
    
    // Validate users are different
    if (senderId == receiverId) {
        result.message = "Cannot challenge yourself";
        return result;
    }
    
    // Check if sender already has active challenge
    QueryResult existingSent = db.queryPrepared(
        "SELECT challenge_id FROM challenges WHERE sender_id = ? AND status = 'pending';",
        {std::to_string(senderId)}
    );
    
    if (!existingSent.empty()) {
        result.message = "You already have an active challenge";
        return result;
    }
    
    // Check if receiver already has pending challenge from anyone
    QueryResult existingReceived = db.queryPrepared(
        "SELECT challenge_id FROM challenges WHERE receiver_id = ? AND status = 'pending';",
        {std::to_string(receiverId)}
    );
    
    if (!existingReceived.empty()) {
        result.message = "Target user already has a pending challenge";
        return result;
    }
    
    // Check if receiver exists and is online
    QueryResult receiverCheck = db.queryPrepared(
        "SELECT player_id, status FROM players WHERE player_id = ?;",
        {std::to_string(receiverId)}
    );
    
    if (receiverCheck.empty()) {
        result.message = "Target user not found";
        return result;
    }
    
    // Create challenge
    int64_t now = getCurrentTimestamp();
    int64_t expiresAt = now + CHALLENGE_TTL_MS;
    
    bool inserted = db.executePrepared(
        "INSERT INTO challenges (sender_id, receiver_id, status, created_at, expires_at) "
        "VALUES (?, ?, 'pending', ?, ?);",
        {std::to_string(senderId), std::to_string(receiverId),
         std::to_string(now), std::to_string(expiresAt)}
    );
    
    if (!inserted) {
        result.message = "Failed to create challenge";
        return result;
    }
    
    int64_t challengeId = db.getLastInsertId();
    
    result.success = true;
    result.challengeId = static_cast<uint32_t>(challengeId);
    result.message = "Challenge sent successfully";
    
    std::cout << "[ChallengeLogic] Challenge sent: id=" << challengeId
              << " sender=" << senderId << " receiver=" << receiverId << "\n";
    
    return result;
}

ChallengeResult ChallengeLogic::cancelChallenge(uint32_t challengeId, uint32_t senderId) {
    ChallengeResult result{false, "", challengeId};
    
    // Check if challenge exists and belongs to sender
    QueryResult challengeCheck = db.queryPrepared(
        "SELECT challenge_id, sender_id, status FROM challenges WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (challengeCheck.empty()) {
        result.message = "Challenge not found";
        return result;
    }
    
    if (std::stoul(challengeCheck[0]["sender_id"]) != senderId) {
        result.message = "Not authorized to cancel this challenge";
        return result;
    }
    
    std::string status = challengeCheck[0]["status"];
    if (status != "pending") {
        result.message = "Challenge already " + status;
        return result;
    }
    
    // Update status to cancelled
    bool updated = db.executePrepared(
        "UPDATE challenges SET status = 'cancelled' WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (!updated) {
        result.message = "Failed to cancel challenge";
        return result;
    }
    
    result.success = true;
    result.message = "Challenge cancelled successfully";
    
    std::cout << "[ChallengeLogic] Challenge cancelled: id=" << challengeId << "\n";
    
    return result;
}

ChallengeResult ChallengeLogic::acceptChallenge(uint32_t challengeId, uint32_t receiverId) {
    ChallengeResult result{false, "", challengeId};
    
    // Check if challenge exists and is for this receiver
    QueryResult challengeCheck = db.queryPrepared(
        "SELECT challenge_id, sender_id, receiver_id, status, expires_at FROM challenges WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (challengeCheck.empty()) {
        result.message = "Challenge not found";
        return result;
    }
    
    if (std::stoul(challengeCheck[0]["receiver_id"]) != receiverId) {
        result.message = "Not authorized to accept this challenge";
        return result;
    }
    
    std::string status = challengeCheck[0]["status"];
    if (status != "pending") {
        result.message = "Challenge already " + status;
        return result;
    }
    
    // Check if expired
    int64_t expiresAt = std::stoll(challengeCheck[0]["expires_at"]);
    if (getCurrentTimestamp() >= expiresAt) {
        // Mark as expired
        db.executePrepared(
            "UPDATE challenges SET status = 'expired' WHERE challenge_id = ?;",
            {std::to_string(challengeId)}
        );
        result.message = "Challenge has expired";
        return result;
    }
    
    // Update status to accepted
    bool updated = db.executePrepared(
        "UPDATE challenges SET status = 'accepted' WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (!updated) {
        result.message = "Failed to accept challenge";
        return result;
    }
    
    result.success = true;
    result.message = "Challenge accepted successfully";
    
    std::cout << "[ChallengeLogic] Challenge accepted: id=" << challengeId << "\n";
    
    // TODO: Create room logic should follow here (CreatingRoom → InRoom states)
    
    return result;
}

ChallengeResult ChallengeLogic::rejectChallenge(uint32_t challengeId, uint32_t receiverId) {
    ChallengeResult result{false, "", challengeId};
    
    // Check if challenge exists and is for this receiver
    QueryResult challengeCheck = db.queryPrepared(
        "SELECT challenge_id, sender_id, receiver_id, status FROM challenges WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (challengeCheck.empty()) {
        result.message = "Challenge not found";
        return result;
    }
    
    if (std::stoul(challengeCheck[0]["receiver_id"]) != receiverId) {
        result.message = "Not authorized to reject this challenge";
        return result;
    }
    
    std::string status = challengeCheck[0]["status"];
    if (status != "pending") {
        result.message = "Challenge already " + status;
        return result;
    }
    
    // Update status to rejected
    bool updated = db.executePrepared(
        "UPDATE challenges SET status = 'rejected' WHERE challenge_id = ?;",
        {std::to_string(challengeId)}
    );
    
    if (!updated) {
        result.message = "Failed to reject challenge";
        return result;
    }
    
    result.success = true;
    result.message = "Challenge rejected successfully";
    
    std::cout << "[ChallengeLogic] Challenge rejected: id=" << challengeId << "\n";
    
    return result;
}

void ChallengeLogic::expireOldChallenges() {
    int64_t now = getCurrentTimestamp();
    
    // Find expired challenges that are still pending
    QueryResult expiredChallenges = db.queryPrepared(
        "SELECT challenge_id FROM challenges WHERE status = 'pending' AND expires_at <= ?;",
        {std::to_string(now)}
    );
    
    if (expiredChallenges.empty()) {
        return;
    }
    
    // Update all to expired
    bool updated = db.executePrepared(
        "UPDATE challenges SET status = 'expired' WHERE status = 'pending' AND expires_at <= ?;",
        {std::to_string(now)}
    );
    
    if (updated) {
        std::cout << "[ChallengeLogic] Expired " << expiredChallenges.size() << " challenges\n";
    }
}

std::optional<ChallengeInfo> ChallengeLogic::getActiveChallenge(uint32_t userId) {
    // Get active challenge where user is sender
    QueryResult result = db.queryPrepared(
        "SELECT challenge_id, sender_id, receiver_id, status, created_at, expires_at "
        "FROM challenges WHERE sender_id = ? AND status = 'pending';",
        {std::to_string(userId)}
    );
    
    if (result.empty()) {
        return std::nullopt;
    }
    
    ChallengeInfo info;
    info.challengeId = std::stoul(result[0]["challenge_id"]);
    info.senderId = std::stoul(result[0]["sender_id"]);
    info.receiverId = std::stoul(result[0]["receiver_id"]);
    info.status = result[0]["status"];
    info.createdAt = std::stoll(result[0]["created_at"]);
    info.expiresAt = std::stoll(result[0]["expires_at"]);
    
    return info;
}

std::optional<ChallengeInfo> ChallengeLogic::getPendingChallengeToUser(uint32_t userId) {
    // Get pending challenge where user is receiver
    QueryResult result = db.queryPrepared(
        "SELECT challenge_id, sender_id, receiver_id, status, created_at, expires_at "
        "FROM challenges WHERE receiver_id = ? AND status = 'pending';",
        {std::to_string(userId)}
    );
    
    if (result.empty()) {
        return std::nullopt;
    }
    
    ChallengeInfo info;
    info.challengeId = std::stoul(result[0]["challenge_id"]);
    info.senderId = std::stoul(result[0]["sender_id"]);
    info.receiverId = std::stoul(result[0]["receiver_id"]);
    info.status = result[0]["status"];
    info.createdAt = std::stoll(result[0]["created_at"]);
    info.expiresAt = std::stoll(result[0]["expires_at"]);
    
    return info;
}
