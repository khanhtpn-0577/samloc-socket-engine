-- ================================================================
-- Migration 003: Auth and Challenges
-- ================================================================

-- Add password_hash column to players (renaming password)
ALTER TABLE players ADD COLUMN password_hash TEXT;

-- Copy existing passwords to password_hash
UPDATE players SET password_hash = password WHERE password_hash IS NULL;

-- Create sessions table for persistent session tokens
CREATE TABLE IF NOT EXISTS sessions (
    session_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    token TEXT NOT NULL UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL,
    FOREIGN KEY(user_id) REFERENCES players(player_id) ON DELETE CASCADE
);

CREATE INDEX idx_sessions_token ON sessions(token);
CREATE INDEX idx_sessions_user_id ON sessions(user_id);
CREATE INDEX idx_sessions_expires_at ON sessions(expires_at);

-- Create challenges table
CREATE TABLE IF NOT EXISTS challenges (
    challenge_id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender_id INTEGER NOT NULL,
    receiver_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'pending',  -- pending | accepted | rejected | cancelled | expired
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL,
    FOREIGN KEY(sender_id) REFERENCES players(player_id) ON DELETE CASCADE,
    FOREIGN KEY(receiver_id) REFERENCES players(player_id) ON DELETE CASCADE
);

CREATE INDEX idx_challenges_sender ON challenges(sender_id);
CREATE INDEX idx_challenges_receiver ON challenges(receiver_id);
CREATE INDEX idx_challenges_status ON challenges(status);
CREATE INDEX idx_challenges_expires_at ON challenges(expires_at);
