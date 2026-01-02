-- ================================================================
-- 🎯 SAMPLE DATA for SamLoc Engine
-- ================================================================

PRAGMA foreign_keys = ON;

-------------------------------------------------
-- 1. Players
-------------------------------------------------
INSERT INTO players (username, password, display_name, balance, status) VALUES 
('admin', '123456', 'Admin System', 999999999, 'ONLINE'),
('player1', '123456', 'Thần Bài 1', 5000000, 'ONLINE'),
('player2', '123456', 'Đỗ Thánh 2', 5000000, 'ONLINE'),
('player3', '123456', 'Cao Thủ 3', 5000000, 'OFFLINE'),
('player4', '123456', 'Gà Mờ 4', 5000000, 'OFFLINE');


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
INSERT INTO rooms (room_code, room_name, room_type, bet_amount, is_private, status, max_players, current_players, created_by) VALUES
(1001, 'Bình Dân 1', 'dat_cuoc', 100000, 0, 'waiting', 4, 0, 1),
(1002, 'Tập Sự 1',   'dat_cuoc', 200000, 0, 'waiting', 4, 0, 1),
(1003, 'Đại Gia 1',  'dat_cuoc', 500000, 0, 'waiting', 4, 0, 1),
(1004, 'VIP Pro 1',  'dat_cuoc', 1000000, 0, 'waiting', 4, 0, 1);

-- == PHÒNG ĐẾM LÁ (Ăn thua theo lá bài) ==
INSERT INTO rooms (room_code, room_name, room_type, bet_amount, is_private, status, max_players, current_players, created_by) VALUES
(2001, 'Đếm Lá Vui',  'dem_la', 5000, 0, 'waiting', 4, 0, 1),
(2002, 'Đếm Lá Sát Phạt', 'dem_la', 5000, 0, 'waiting', 4, 0, 1),
(2003, 'Đếm Lá Cao Cấp', 'dem_la', 5000, 0, 'waiting', 4, 0, 1),
(2004, 'Đếm Lá Sinh Tử', 'dem_la', 5000, 0, 'waiting', 4, 0, 1);

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
INSERT INTO penalties (description) VALUES 
('Thối Heo'),
('Chặt Heo'),
('Báo Sâm Thất Bại'),
('Chặt Chồng'),
('Thối Tứ Quý');

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
INSERT INTO cards (suit, rank, display_name, order_value) VALUES
-- ===== SPADES ♠ (Bích) =====
('spade','3','3♠',3), ('spade','4','4♠',4), ('spade','5','5♠',5), ('spade','6','6♠',6),
('spade','7','7♠',7), ('spade','8','8♠',8), ('spade','9','9♠',9), ('spade','10','10♠',10),
('spade','J','J♠',11), ('spade','Q','Q♠',12), ('spade','K','K♠',13), ('spade','A','A♠',14), ('spade','2','2♠',15),

-- ===== CLUBS ♣ (Tép/Chuồn) - Thứ tự thứ 2 =====
('club','3','3♣',3), ('club','4','4♣',4), ('club','5','5♣',5), ('club','6','6♣',6),
('club','7','7♣',7), ('club','8','8♣',8), ('club','9','9♣',9), ('club','10','10♣',10),
('club','J','J♣',11), ('club','Q','Q♣',12), ('club','K','K♣',13), ('club','A','A♣',14), ('club','2','2♣',15),

-- ===== DIAMONDS ♦ (Rô) - Thứ tự thứ 3 =====
('diamond','3','3♦',3), ('diamond','4','4♦',4), ('diamond','5','5♦',5), ('diamond','6','6♦',6),
('diamond','7','7♦',7), ('diamond','8','8♦',8), ('diamond','9','9♦',9), ('diamond','10','10♦',10),
('diamond','J','J♦',11), ('diamond','Q','Q♦',12), ('diamond','K','K♦',13), ('diamond','A','A♦',14), ('diamond','2','2♦',15),

-- ===== HEARTS ♥ (Cơ) - Thứ tự lớn nhất =====
('heart','3','3♥',3), ('heart','4','4♥',4), ('heart','5','5♥',5), ('heart','6','6♥',6),
('heart','7','7♥',7), ('heart','8','8♥',8), ('heart','9','9♥',9), ('heart','10','10♥',10),
('heart','J','J♥',11), ('heart','Q','Q♥',12), ('heart','K','K♥',13), ('heart','A','A♥',14), ('heart','2','2♥',15);
