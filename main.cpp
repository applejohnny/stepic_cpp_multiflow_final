#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include "server.hpp"

using namespace std;

int main(int argc, char ** argv) {

    string host;
    uint16_t port = 0;
    string home_dir;

    int opt;
    while ( (opt = getopt(argc, argv, "h:p:d:")) != -1 ) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = (uint16_t)atoi(optarg);
                break;
            case 'd':
                home_dir = optarg;
                break;
            default:
                break;
        }
    }

    Server server(host, port, home_dir);
    server.setDebug(false);
    server.run(false);

    return 0;
}