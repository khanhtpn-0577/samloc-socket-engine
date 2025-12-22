#include <iostream>
#include <vector>
#include <unordered_map>
#include <poll.h>

#include "net/server_socket.h"
#include "handler/connection/connection_handler.h"

int main(){
    ServerSocket server(5000);
    if(!server.listen()) return 1;

    std::vector<pollfd> pollFds; //mang luu tru cac file descriptor(fd, events-input cho kernel(POLLIN-co the doc, POLLOUT-co the ghi), revent - output tra ve tu kernel(event nay dang gap chuyen gi?))
    std::unordered_map<int, ConnectionHandler*> handlers; //bang anh xa clientFd ->instance ConnectionHandler

    pollFds.push_back({server.getFd(), POLLIN, 0}); //day listenfd vao mang pollFds

    std::cout << "[Server] Poll loop started\n";

    while (true){
        int ready = poll(pollFds.data(), pollFds.size(),-1); //khong co timeout, kernel sleep cho den khi co it nhat 1 fd co event
        if(ready < 0){
            perror("poll");
            break;
        }

        for (size_t i = 0; i < pollFds.size(); ++i){
            if (!(pollFds[i].revents & POLLIN)) continue; //neu fd[i] khong readable thi bo qua

            int fd = pollFds[i].fd;

            //new connection
            if (fd == server.getFd()){ //neu fd readable hien tai la listenfd --> co client moi --> append vao pollFds
                int clientFd = server.acceptClient();
                if(clientFd >= 0){ //neu tao fd cho client thanh cong --> ghi vao pollFds
                    pollFds.push_back({clientFd, POLLIN, 0});
                    handlers[clientFd] = new ConnectionHandler(clientFd); //map clientfd va handler

                    std::cout << "[Server] New client fd = " << clientFd << std::endl;
                }
            }

            //client data
            else {
                auto it = handlers.find(fd); //tim handler ung voi fd, handler la 1 object binh thuong, khong tao thread moi
                if (it ==handlers.end()) continue; //neu khong tim thay handler --> continue

                bool ok = it->second->onReadable(); //it->second la vi tri cua ConnectionHandler trong pair
                if(!ok){
                    std::cout << "[Server] Client fd =" << fd << " disconnected\n";

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