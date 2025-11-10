-- ================================================================
-- 🎯 SAMPLE DATA for SamLoc Engine
-- ================================================================

PRAGMA foreign_keys = ON;

-------------------------------------------------
-- 1. Players
-------------------------------------------------
INSERT INTO players (username, password, display_name, balance, status)
VALUES
('alice', '123456', 'Alice', 1500, 'online'),
('bob', '123456', 'Bob', 1200, 'offline'),
('charlie', '123456', 'Charlie', 800, 'online'),
('david', '123456', 'David', 2000, 'online');

-------------------------------------------------
-- 2. Friend Requests
-------------------------------------------------
INSERT INTO friend_requests (sender_id, receiver_id, status)
VALUES
(1, 2, 'accepted'),
(2, 3, 'pending');

-------------------------------------------------
-- 3. Friends (Alice - Bob)
-------------------------------------------------
INSERT INTO friends (player_id, friend_id)
VALUES
(1, 2),
(2, 1);

-------------------------------------------------
-- 4. Rooms
-------------------------------------------------
INSERT INTO rooms (room_code, room_name, room_type, bet_amount, is_private, status, max_players, current_players, created_by)
VALUES
(1001, 'Phòng 1001', 'dat_cuoc', 100, 0, 'waiting', 4, 2, 1),
(2001, 'Phòng 2001', 'dem_la', 0, 1, 'playing', 4, 4, 2);

-------------------------------------------------
-- 5. Room Players
-------------------------------------------------
INSERT INTO room_players (room_id, player_id, position)
VALUES
(1, 1, 1),
(1, 2, 2),
(2, 3, 1),
(2, 4, 2);

-------------------------------------------------
-- 6. Games
-------------------------------------------------
INSERT INTO games (room_id, game_type, start_time, end_time, winner_id, status)
VALUES
(1, 'sam_loc', datetime('now', '-5 minutes'), datetime('now'), 1, 'finished'),
(2, 'mat_chuoc', datetime('now', '-2 minutes'), NULL, NULL, 'playing');

-------------------------------------------------
-- 7. Game Results
-------------------------------------------------
INSERT INTO game_results (game_id, player_id, ranking, card_left_count, reward)
VALUES
(1, 1, 1, 0, 300),
(1, 2, 2, 5, -100);

-------------------------------------------------
-- 8. Penalties
-------------------------------------------------
INSERT INTO penalties (description)
VALUES
('Chặt heo sai luật'),
('Bỏ lượt quá thời gian');

-------------------------------------------------
-- 9. Chats
-------------------------------------------------
INSERT INTO chats (room_id, sender_id, message)
VALUES
(1, 1, 'Chơi vui nhé!'),
(1, 2, 'Ok luôn!');

-------------------------------------------------
-- 10. Game Moves
-------------------------------------------------
INSERT INTO game_moves (user_id, game_id, card_played, move_type)
VALUES
(1, 1, '{"cards": ["A♠", "A♥"]}', 'play'),
(2, 1, '{"cards": []}', 'pass');

-------------------------------------------------
-- 11. Cards
-------------------------------------------------
INSERT INTO cards (suit, rank, display_name, order_value)
VALUES
('spade', 'A', 'A♠', 13),
('heart', 'A', 'A♥', 13),
('diamond', '2', '2♦', 15),
('club', 'K', 'K♣', 12),
('heart', '10', '10♥', 10);
