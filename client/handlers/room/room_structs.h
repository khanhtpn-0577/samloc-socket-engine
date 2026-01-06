#pragma once
#include <string>
#include <vector>

struct RoomInfo {
    int id;
    std::string name;
    std::string type;
    int bet;
    int currentPlayers;
    int maxPlayers;
    std::string status;
};

struct RoomMember {
    int id;
    std::string name;
    long long balance;
    bool isReady;
    bool isHost; 
    int handSize; 
    std::vector<int> hand;
};