#include <iostream>
#include <vector>
#include <unordered_map>
#include <poll.h>
#include <fcntl.h>
#include <filesystem>
#include <chrono>
#include <signal.h>
#include "net/server_socket.h"
#include "handler/connection/connection_handler.h"
#include "handler/game/game_handler.h"
#include "db/database.h"

int main(){
    signal(SIGPIPE, SIG_IGN);
    const std::string dbPath = "../../samloc.db";
    Database db(dbPath);
    std::cout << "[Server] Database initialized: " << dbPath << "\n";

    namespace fs = std::filesystem;
    bool dbExists = fs::exists(dbPath) && fs::file_size(dbPath) > 0;
    if (!dbExists) {
        db.initSchemaFromFile("server/db/schema.sql");
        db.loadSampleDataFromFile("server/db/sample_data.sql");
    }

    GameHandler::instance().init(db);

    ServerSocket server(5000);
    if(!server.listen()) return 1;

    std::vector<pollfd> pollFds;
    std::unordered_map<int, ConnectionHandler*> handlers;

    pollFds.push_back({server.getFd(), POLLIN, 0});
    std::cout << "[Server] Poll loop started\n";

    while (true){
        // ⏱ poll timeout 50ms
        int ready = poll(pollFds.data(), pollFds.size(), 50);

        if(ready < 0){
            perror("poll");
            break;
        }

        GameHandler::instance().updateLoop();

        for (size_t i = 0; i < pollFds.size(); ++i){
            if (!(pollFds[i].revents & POLLIN)) continue;

            int fd = pollFds[i].fd;

            if (fd == server.getFd()){
                int clientFd = server.acceptClient();
                if(clientFd >= 0){
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

                    pollFds.push_back({clientFd, POLLIN, 0});
                    handlers[clientFd] = new ConnectionHandler(clientFd, db);

                    std::cout << "[Server] New client fd = " << clientFd << "\n";
                }
            }
            else {
                auto it = handlers.find(fd);
                if (it == handlers.end()) continue;

                bool ok = it->second->onReadable();
                if(!ok){
                    it->second->closeConnection();
                    delete it->second;
                    handlers.erase(it);

                    pollFds.erase(pollFds.begin() + i);
                    --i;
                }
            }
        }
    }
}
