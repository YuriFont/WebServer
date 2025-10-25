#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServer.hpp"
#include "Config.hpp"
#include <sys/epoll.h>
#include "HttpStatus.hpp"

class HttpRequest;

class Server {
    public:
        Server(const Config &config);
        ~Server();
        void start();

    private:
        int server_fd;
        int epoll_fd;
        epoll_event events[10];
        void initSocket();
        void startListenServer();
        void createEpoll();
        void eventLoop();
        void handleNewConnection();
        void handleClientRequest(int client_fd);
        const Config& _config;
        const Location& findLocation(HttpRequest& request);
        std::string fileToString(const Location& location, const std::string& nameFile);
};

#endif