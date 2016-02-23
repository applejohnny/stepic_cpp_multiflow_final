#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <streambuf>

#include "server.hpp"

#define REQUEST_BUFFER_SIZE 1024

void Server::demonize() {
    // форкаемся, чтобы демонезироваться
    //
    pid_t pid = fork();
    if (pid < 0) {
        perror("pid");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        // закрываем родительский процесс
        exit(EXIT_SUCCESS);
    }

    pid_t sid = setsid();
    if (sid < 0) {
        perror("sid");
        exit(EXIT_FAILURE);
    }

    // закрываем все дескрипторы
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void Server::run(bool demonize) {
    int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(this->port);
    inet_pton(AF_INET, this->host.c_str(), &(sock_addr.sin_addr));

    int res;
    res = bind(master_socket, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (res < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    res = listen(master_socket, SOMAXCONN);
    if (res == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (this->debug) {
        std::cout << "server is running on: " << this->host << ":" << this->port << std::endl;
    }

    if (demonize) {
        this->demonize();
    }

    while (true) {
        int socket = accept(master_socket, 0, 0);
        std::thread th(&Server::handle_request, *this, socket);
        th.detach();
    }
}

void Server::handle_request(int socket) {
    char buf[REQUEST_BUFFER_SIZE];
    recv(socket, buf, REQUEST_BUFFER_SIZE, 0);

    std::string req;
    req.append(buf);

    // обрежим запрос до первой строчки
    size_t pos;
    if ( (pos = req.find('\n')) != std::string::npos) {
        if (pos > 0 && req[pos - 1] == '\r') pos--;
        req = req.substr(0, pos);
    }

    std::string res;
    do {
        // 500 некорректный или неподдерживаемый запрос
        if (
                req.substr(0, 4) != "GET "
                || req.substr(req.length() - 9, 8) != " HTTP/1.")
        {
            res = "HTTP/1.0 500 Internal Server Error\n"
                    "Content-Type: text/html\n\n";
            break;
        }

        // вырезаем путь из первой строчки запроса
        std::string path = req.substr(4, req.length() - 13);
        // уберем параметры
        if ( (pos = path.find("?")) != std::string::npos ) {
            path = path.substr(0, pos);
        }

        std::ifstream file(this->home_dir + path);

//        std::string p = this->home_dir + path + '\n';
//        send(socket, p.c_str(), p.size(), 0);


        struct stat file_stat;
        stat((this->home_dir + path).c_str(), &file_stat);

        // 404
        if (
                // не смогли открыть файл
                !file.is_open()
                // это не файл
                || !(file_stat.st_mode & S_IFREG)
        ) {
            res = "HTTP/1.0 404 NOT FOUND\n"
                    "Content-Type: text/html\n\n";
            break;
        }

        std::string str_file(
                (std::istreambuf_iterator<char>(file)),
                (std::istreambuf_iterator<char>())
        );

        file.close();

        // 200 OK
        res = "HTTP/1.0 200 OK\n"
                "Content-length: " + std::to_string(str_file.size()) +  "\n"
                "Connection: close\n"
                "Content-Type: text/html\n\n"
                + str_file;

    } while (false);

    send(socket, res.c_str(), res.size(), 0);
    shutdown(socket, SHUT_RDWR);
    close(socket);
}