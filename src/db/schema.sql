-- ================================================================
-- 🎮 GAME MẠT CHƯỢC – SÂM LỐC DATABASE DESIGN
-- ================================================================

PRAGMA foreign_keys = ON;

-------------------------------------------------
-- 1. Players
-------------------------------------------------
CREATE TABLE IF NOT EXISTS players (
    player_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT NOT NULL UNIQUE,
    password        TEXT NOT NULL,
    display_name    TEXT,
    balance         REAL DEFAULT 0,
    status          TEXT,
    created_at      TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-------------------------------------------------
-- 2. Friend Requests
-------------------------------------------------
CREATE TABLE IF NOT EXISTS friend_requests (
    friend_request_id INTEGER PRIMARY KEY AUTOINCREMENT,
    sender_id   INTEGER NOT NULL,
    receiver_id INTEGER NOT NULL,
    status      TEXT,
    sent_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(sender_id) REFERENCES players(player_id) ON DELETE CASCADE,
    FOREIGN KEY(receiver_id) REFERENCES players(player_id) ON DELETE CASCADE
);

-------------------------------------------------
-- 3. Friends
-------------------------------------------------
CREATE TABLE IF NOT EXISTS friends (
    player_id   INTEGER NOT NULL,
    friend_id   INTEGER NOT NULL,
    created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (player_id, friend_id),
    FOREIGN KEY(player_id) REFERENCES players(player_id) ON DELETE CASCADE,
    FOREIGN KEY(friend_id) REFERENCES players(player_id) ON DELETE CASCADE
);

-------------------------------------------------
-- 4. Rooms
-------------------------------------------------
CREATE TABLE IF NOT EXISTS rooms (
    room_id          INTEGER PRIMARY KEY AUTOINCREMENT,
    room_code        INTEGER,
    room_name        TEXT,
    room_type        TEXT,   -- dem_la | dat_cuoc
    bet_amount       INTEGER,
    is_private       BOOLEAN DEFAULT 0,
    status           TEXT,   -- waiting | playing
    max_players      INTEGER,
    current_players  INTEGER,
    created_by       INTEGER NOT NULL,
    created_at       TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(created_by) REFERENCES players(player_id)
);

-------------------------------------------------
-- 5. Room Players
-------------------------------------------------
CREATE TABLE IF NOT EXISTS room_players (
    room_id     INTEGER NOT NULL,
    player_id   INTEGER NOT NULL,
    join_time   TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    leave_time  TIMESTAMP,
    position    INTEGER,
    PRIMARY KEY (room_id, player_id),
    FOREIGN KEY(room_id) REFERENCES rooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY(player_id) REFERENCES players(player_id) ON DELETE CASCADE
);

-------------------------------------------------
-- 6. Games
-------------------------------------------------
CREATE TABLE IF NOT EXISTS games (
    game_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    room_id     INTEGER NOT NULL,
    game_type   TEXT,      -- sam_loc | mat_chuoc
    start_time  TIMESTAMP,
    end_time    TIMESTAMP,
    winner_id   INTEGER,
    status      TEXT,      -- waiting | playing | finished
    FOREIGN KEY(room_id) REFERENCES rooms(room_id),
    FOREIGN KEY(winner_id) REFERENCES players(player_id)
);

-------------------------------------------------
-- 7. Game Results
-------------------------------------------------
CREATE TABLE IF NOT EXISTS game_results (
    game_result_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    game_id           INTEGER NOT NULL,
    player_id         INTEGER NOT NULL,
    ranking           INTEGER,
    card_left_count   INTEGER,
    reward            REAL,
    penalty_id        INTEGER,
    FOREIGN KEY(game_id) REFERENCES games(game_id),
    FOREIGN KEY(player_id) REFERENCES players(player_id),
    FOREIGN KEY(penalty_id) REFERENCES penalties(penalty_id)
);

-------------------------------------------------
-- 8. Penalties
-------------------------------------------------
CREATE TABLE IF NOT EXISTS penalties (
    penalty_id    INTEGER PRIMARY KEY AUTOINCREMENT,
    description   TEXT
);

-------------------------------------------------
-- 9. Chats
-------------------------------------------------
CREATE TABLE IF NOT EXISTS chats (
    chat_id     INTEGER PRIMARY KEY AUTOINCREMENT,
    room_id     INTEGER NOT NULL,
    sender_id   INTEGER NOT NULL,
    message     TEXT,
    sent_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(room_id) REFERENCES rooms(room_id),
    FOREIGN KEY(sender_id) REFERENCES players(player_id)
);

-------------------------------------------------
-- 10. Game Moves
-------------------------------------------------
CREATE TABLE IF NOT EXISTS game_moves (
    move_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id       INTEGER NOT NULL,
    game_id       INTEGER NOT NULL,
    created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    card_played   TEXT,  -- JSON string
    move_type     TEXT,  -- play | pass | win
    FOREIGN KEY(game_id) REFERENCES games(game_id),
    FOREIGN KEY(user_id) REFERENCES players(player_id)
);

-------------------------------------------------
-- 11. Cards
-------------------------------------------------
CREATE TABLE IF NOT EXISTS cards (
    card_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    suit          TEXT NOT NULL,   -- spade | heart | diamond | club
    rank          TEXT NOT NULL,   -- 2–10, J, Q, K, A
    display_name  TEXT,
    order_value   INTEGER
);

-------------------------------------------------
-- 12. Migrations (quản lý lịch sử cập nhật DB)
-------------------------------------------------
CREATE TABLE IF NOT EXISTS migrations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,           -- tên file migration
    applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
