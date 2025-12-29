-- ================================================================
-- Migration: Rename chats -> room_chats & add private_chats
-- ================================================================

PRAGMA foreign_keys = OFF;
BEGIN TRANSACTION;

-------------------------------------------------
-- 1. Rename table chats -> room_chats
-------------------------------------------------

ALTER TABLE chats RENAME TO room_chats;

-------------------------------------------------
-- 2. Create private_chats table
-------------------------------------------------
CREATE TABLE IF NOT EXISTS private_chats (
    chat_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    sender_id    INTEGER NOT NULL,
    receiver_id  INTEGER NOT NULL,
    message      TEXT,
    sent_at      TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY(sender_id) REFERENCES players(player_id) ON DELETE CASCADE,
    FOREIGN KEY(receiver_id) REFERENCES players(player_id) ON DELETE CASCADE
);

COMMIT;