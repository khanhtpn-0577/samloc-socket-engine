#pragma once

// Game timing configuration (seconds)
constexpr int GAME_READY_DELAY_SECONDS = 3;        // sau khi all ready -> schedule start sau 3s
constexpr int GAME_BIDDING_TIMEOUT_SECONDS = 15;    // thời gian để báo sâm (bao sam)
constexpr int GAME_TURN_TIMEOUT_SECONDS = 20;      // thời gian mỗi lượt đánh
constexpr int GAME_UPDATE_LOOP_MS = 200;           // nếu start thread updateLoop thì chạy mỗi 200ms
