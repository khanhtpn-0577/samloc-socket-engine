-- ================================================================
-- 🎲 Lucky Spin Feature Migration
-- ================================================================
PRAGMA foreign_keys = ON;

-------------------------------------------------
-- 1. Lucky Spin Rewards (Danh sách phần thưởng)
-------------------------------------------------
CREATE TABLE IF NOT EXISTS lucky_spin_rewards (
    reward_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    reward_name    TEXT NOT NULL,
    reward_type    TEXT NOT NULL,      -- coin | extra_spin
    reward_amount  INTEGER NOT NULL,   -- số coin hoặc số lượt
    probability    REAL NOT NULL       -- %
);

-------------------------------------------------
-- Insert 8 phần thưởng mặc định
-------------------------------------------------
INSERT INTO lucky_spin_rewards (reward_name, reward_type, reward_amount, probability)
VALUES
('1k coin',      'coin',        1000,     25.0),
('10k coin',     'coin',        10000,    20.0),
('20k coin',     'coin',        20000,    15.0),
('50k coin',     'coin',        50000,    10.0),
('1 Extra Spin', 'extra_spin',  1,        10.0),
('100k coin',    'coin',        100000,   8.0),
('200k coin',    'coin',        200000,   5.0),
('1M coin',      'coin',        1000000,  2.0);

-------------------------------------------------
-- 2. Lucky Spin Daily (Theo dõi lượt quay của từng người chơi)
-------------------------------------------------
CREATE TABLE IF NOT EXISTS lucky_spin_daily (
    player_id       INTEGER PRIMARY KEY,
    free_spins      INTEGER DEFAULT 1,         -- miễn phí mỗi ngày
    extra_spins     INTEGER DEFAULT 0,         -- mua hoặc được thưởng
    last_reset      DATE,                      -- ngày reset
    FOREIGN KEY(player_id) REFERENCES players(player_id) ON DELETE CASCADE
);

-------------------------------------------------
-- 3. Lucky Spin History (Lịch sử mỗi lần quay)
-------------------------------------------------
CREATE TABLE IF NOT EXISTS lucky_spin_history (
    spin_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    player_id      INTEGER NOT NULL,
    reward_id      INTEGER NOT NULL,
    reward_amount  INTEGER NOT NULL,
    spin_time      TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    consumed_type  TEXT NOT NULL,              -- free | extra | new_user | purchased
    FOREIGN KEY(player_id) REFERENCES players(player_id) ON DELETE CASCADE,
    FOREIGN KEY(reward_id) REFERENCES lucky_spin_rewards(reward_id)
);
