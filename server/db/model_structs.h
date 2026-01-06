#pragma once
#include <string>

// Cấu trúc ánh xạ bảng 'players'
struct DBPlayer {
    int id;
    std::string displayName;
    double balance;
    std::string status;
};

// Cấu trúc ánh xạ bảng 'rooms'
struct DBRoom {
    int id;
    std::string type;       // "dat_cuoc" | "dem_la"
    std::string roomName;
    double betAmount;
    int maxPlayers;
    int currentPlayers;
    std::string status;     // "waiting" | "playing"
};

// Cấu trúc ánh xạ bảng 'cards'
struct DBCard {
    int id;
    std::string suit;       // "spade", "club", "diamond", "heart"
    std::string rank;       // "3", "4", ..., "A", "2"
    int orderValue;         // 3..15 (Dùng để so sánh lớn nhỏ)

    // Operator để so sánh 2 lá bài (dùng trong std::find, std::remove)
    bool operator==(const DBCard& other) const { 
        return id == other.id; 
    }
    
    // Operator so sánh nhỏ hơn (dùng để sort bài trên tay)
    bool operator<(const DBCard& other) const {
        return orderValue < other.orderValue;
    }
};