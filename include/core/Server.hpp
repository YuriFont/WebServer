#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServer.hpp"
#include "../config/Config.hpp"
#include <sys/epoll.h>
#include "../http/HttpStatus.hpp"
#include "Client.hpp"

class HttpRequest;

class Server {
    public:
        Server(const Config &config);
        ~Server();
        void start();
        void shutdown();

    private:
        const Config &_config;
        int epoll_fd;
        epoll_event events[10];
        std::vector<int> server_fds;
        std::map<int, ServerConfig> server_by_fd;
        std::map<int, ServerConfig*> client_server;
        std::map<int, Client> clients;
        bool _running;
        
        void initAllSockets();
        void registerSocketsInEpoll();
        void handleNewConnection(int server_fd);
        void handleClientRequest(int client_fd);
        const Location &findLocation(ServerConfig *serverCfg, HttpRequest &request);
        void eventLoop();
        void removeClient(int client_fd);
        void readClientBuffer(const int& client_fd, char* buffer, size_t bufSize, int& bytes);
        void addBuffer(Client& client, char* buffer, int& bytes);
        IMethodHandler* buildMethodHandler(Client& client, int &client_fd);
        void sendResponse(const int &client_fd, Client& client);
        bool removeMethodHandler(Client& client, HttpResponse& resp);
        void finalizeClientConnection(const int &client_fd, Client& client, const bool& closeConnection);
};

#endif