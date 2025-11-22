#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServer.hpp"
#include "Config.hpp"
#include <sys/epoll.h>
#include "HttpStatus.hpp"
#include "Client.hpp"

class HttpRequest;

class Server {
    public:
        Server(const Config &config);
        ~Server();
        void start();

    private:
        const Config &_config;
        int epoll_fd;
        epoll_event events[10];
        std::vector<int> server_fds;
        std::map<int, ServerConfig> server_by_fd;
        std::map<int, ServerConfig*> client_server;
        std::map<int, Client> clients;
        
        void initAllSockets();
        void registerSocketsInEpoll();
        void handleNewConnection(int server_fd);
        void handleClientRequest(int client_fd);
        const Location &findLocation(ServerConfig *serverCfg, HttpRequest &request);
        void eventLoop();
};

#endif