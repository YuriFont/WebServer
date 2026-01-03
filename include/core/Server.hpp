#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServer.hpp"
#include "../config/Config.hpp"
#include <sys/epoll.h>
#include "../http/HttpStatus.hpp"
#include "Client.hpp"
#include "../handlers/CgiProcess.hpp"

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
        epoll_event events[64];
        std::vector<int> server_fds;
        std::map<int, ServerConfig> server_by_fd;
        std::map<int, ServerConfig*> client_server;
        std::map<int, Client> clients;
        std::map<int, CgiProcess*> _cgiByFd;
        bool _running;
        const int _CLIENT_TIMEOUT;
        
        void initAllSockets();
        void registerSocketsInEpoll();
        void handleNewConnection(int server_fd);
        void sendResponseClient(int client_fd);
        void handleClientRequest(int client_fd);
        const Location &findLocation(ServerConfig *serverCfg, HttpRequest &request);
        void eventLoop();
        void removeClient(int client_fd);
        void readClientBuffer(const int& client_fd, char* buffer, size_t bufSize, int& bytes);
        void addBuffer(Client& client, char* buffer, int& bytes);
        IMethodHandler* buildMethodHandler(Client& client, int &client_fd);
        void prepareResponse(const int &client_fd, Client& client);
        bool removeMethodHandler(Client& client);
        void finalizeClientConnection(const int &client_fd, Client& client, const bool& closeConnection);
        void logStatusResponse(const int &client_fd, Client& client);
        void logClientDesconected(const int &client_fd);
        void logClienteConected(const int &client_fd);
        void logClienteRequest(const int &client_fd, Client& client);
        void epoll_add(int fd, uint32_t events);
        void finalizeCgiResponse(CgiProcess* cgi);
        void startCgiForClient(Client& client);
        void handleCgiEvent(int fd, uint32_t ev);
        void handleCgiWrite(int fd);
        void handleCgiRead(int fd);
        void removeCgiFd(const int& fd);
        void updateClientActivity(int fd);
        void checkClientTimeouts();
        void updateCgiActivity(int fd);
        void checkCgiTimeouts();
        void killTimedOutCgi(CgiProcess* cgi);
        void prepareBadRequest(Client& client);
        bool handleCgiFailure(Client& client, CgiProcess* cgi);
};

#endif