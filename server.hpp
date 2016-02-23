#pragma once

#include <string>

class Server
{
private:
    std::string host;
    std::uint16_t port;
    std::string home_dir;
    bool debug;

    void handle_request(int socket);
    void demonize();

public:
    Server(std::string host, std::uint16_t port, std::string home_dir) {
        this->host = host;
        this->port = port;
        this->home_dir = home_dir;
    }


    void run(bool demonize = false);

    void setDebug(bool debug_) {
        debug = debug_;
    }
};