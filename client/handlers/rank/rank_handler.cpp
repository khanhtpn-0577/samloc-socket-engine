#include "rank_handler.h"
#include <iostream>

RankHandler::RankHandler(RankLogic& logic, ClientSession& session)
    : rankLogic_(logic), session_(session) {}

void RankHandler::setFriendRankCallback(
    std::function<void(const std::vector<FriendRankInfo>&)> cb
) {
    friendRankCallback_ = std::move(cb);
}

void RankHandler::requestFriendRank() {
    std::cout << "[RankHandler] Requesting friend ranking...\n";
    if (!session_.isLoggedIn()) return;

    rankLogic_.requestFriendRank(session_.userId());
}

void RankHandler::onFriendRankResponse(const Message& message) {
    const std::string& payload = message.payload;

    std::vector<FriendRankInfo> ranks;

    size_t pos = payload.find("\"ranks\"");
    if (pos == std::string::npos) {
        std::cerr << "[RankHandler] Invalid rank payload\n";
        return;
    }

    pos = payload.find("[", pos);
    if (pos == std::string::npos) return;
    ++pos; // skip '['

    while (true) {
        size_t idPos = payload.find("\"id\":", pos);
        if (idPos == std::string::npos) break;

        size_t idStart = idPos + 5;
        size_t idEnd = payload.find(",", idStart);
        uint32_t id =
            static_cast<uint32_t>(
                std::stoul(payload.substr(idStart, idEnd - idStart))
            );

        size_t namePos = payload.find("\"name\":\"", idEnd);
        if (namePos == std::string::npos) break;

        size_t nameStart = namePos + 8;
        size_t nameEnd = payload.find("\"", nameStart);
        std::string name =
            payload.substr(nameStart, nameEnd - nameStart);

        size_t balPos = payload.find("\"balance\":", nameEnd);
        if (balPos == std::string::npos) break;

        size_t balStart = balPos + 10;
        size_t balEnd = payload.find_first_of(",}", balStart);
        long long balance =
            std::stoll(payload.substr(balStart, balEnd - balStart));

        ranks.push_back({id, name, balance});
        pos = balEnd;
    }

    std::cout << "[RankHandler] Received friend ranking: "
              << ranks.size() << " entries\n";

    if (friendRankCallback_) {
        friendRankCallback_(ranks);
    }
}
