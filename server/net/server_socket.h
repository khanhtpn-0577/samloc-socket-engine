#pragma once

class ServerSocket {
public:
    explicit ServerSocket(int port);
    ~ServerSocket();

    bool listen();
    int acceptClient();
    int getFd() const;
    void close();

private:
    bool createSocket();

private:
    int socketFd;
    int port;
    bool listening;
};
