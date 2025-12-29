-- ================================================================
-- Migration: Add status column to private_chats and room_chats
-- ================================================================

PRAGMA foreign_keys = OFF;
BEGIN TRANSACTION;

-------------------------------------------------
-- 1. Add status to private_chats
-------------------------------------------------
ALTER TABLE private_chats
ADD COLUMN status TEXT DEFAULT 'sent';

-------------------------------------------------
-- 2. Add status to room_chats
-------------------------------------------------
ALTER TABLE room_chats
ADD COLUMN status TEXT DEFAULT 'sent';

-- sent(server nhan tin nhan), delivered(da push tin nhan cho receiver), seen (receiver da xem tin nhan)

COMMIT;
PRAGMA foreign_keys = ON;
