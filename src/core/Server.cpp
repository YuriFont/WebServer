#include "../../include/core/WebServer.hpp"
#include "../../include/core/Server.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/handlers/RequestHandler.hpp"
#include "../../include/config/Config.hpp"
#include "../../include/utils/Utils.hpp"

Server::Server(const Config &config) : _config(config), _running(true) {}

Server::~Server() {
    for (size_t i = 0; i < server_fds.size(); i++) {
        close(server_fds[i]);
    }

    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->first);
    }

    close(epoll_fd);
}

void Server::initAllSockets() {
    for (size_t i = 0; i < _config.servers.size(); i++) {
        ServerConfig sc = _config.servers[i];
        int fd = sc.initSocket();
        server_fds.push_back(fd);
        server_by_fd[fd] = sc;
    }
}

void Server::registerSocketsInEpoll() {
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        perror("epoll_create");
        exit(1);
    }

    for (size_t i = 0; i < server_fds.size(); i++) {
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = server_fds[i];
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fds[i], &ev);
    }
}

void Server::handleNewConnection(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) return;
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    Client client(client_fd);
    client_server[client_fd] = &server_by_fd[server_fd];
    clients[client_fd] = client;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &clients[client_fd].getDataEvent());
    std::cout << "New client in the server " << server_by_fd[server_fd].getPort() << std::endl;
}

const Location &Server::findLocation(ServerConfig *serverCfg, HttpRequest &request) {
    const std::map<std::string, Location> &locs = serverCfg->getLocations();

    std::string path = request.getPath();
    std::string match = "/";
    size_t longestMatch = 0;

    for (std::map<std::string, Location>::const_iterator it = locs.begin();
         it != locs.end(); ++it)
    {
        if (path.compare(0, it->first.size(), it->first) == 0 &&
            it->first.size() > longestMatch)
        {
            longestMatch = it->first.size();
            match = it->first;
        }
    }

    return locs.at(match);
}

void Server::removeClient(int client_fd) {
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);
    clients.erase(client_fd);
}

void Server::readClientBuffer(const int& client_fd, char* buffer, size_t bufSize, int& bytes) {
    
    bytes = recv(client_fd, buffer, bufSize, 0);

    if (bytes <= 0) {
        removeClient(client_fd);
        std::cout << "Client disconnected." << std::endl;
    }
}

IMethodHandler* Server::buildMethodHandler(Client& client , int &client_fd) {

    HttpRequest& request = client.getRequest();
    const Location &location = findLocation(client_server[client_fd], request);
    const ServerConfig& serverConfig = *client_server[client_fd];
    return RequestHandler::handle(serverConfig, request, location);
}

bool Server::removeMethodHandler(Client& client, HttpResponse& resp) {
    
    bool closeConnection = resp.isConnectionClose();
    
    delete client.handler;
    client.handler = NULL;
    
    return closeConnection;
}

void Server::finalizeClientConnection(const int &client_fd, Client& client, const bool& closeConnection) {

    if (closeConnection) {
        removeClient(client_fd);
    } else {
        client.cleanData();
    }
}

void Server::sendResponse(const int &client_fd, Client& client) {
    
    HttpResponse& resp = client.handler->getResponse();
    send(client_fd, resp.toString().c_str(), resp.toString().size(), 0);
    bool closeConnection = removeMethodHandler(client, resp);
    finalizeClientConnection(client_fd, client, closeConnection);
}

void Server::addBuffer(Client& client, char* buffer, int& bytes) {

    if (!client.isAllHeaders()) {
        client.addBuffer(std::string(buffer, bytes));
    } else {
        client.addBody(std::string(buffer, bytes));
    }
}

void Server::handleClientRequest(int client_fd) {
    
    char buffer[2048];
    int bytes = 0;

    readClientBuffer(client_fd, buffer, sizeof(buffer), bytes);
    if (bytes <= 0) {
        return ;
    }
    Client& client = clients[client_fd];
    addBuffer(client, buffer, bytes);
    if (!client.isAllHeaders())
        return ;
    if (client.handler == NULL) {
        client.handler = buildMethodHandler(client, client_fd);
    }
    client.handler->handleData(client.getRequest().getBody());
    client.eraseBody();
    if (client.handler->isFinished()) {
        sendResponse(client_fd, client);
    }
    else{
        clients[client_fd].cleanData();
    }
}

void Server::eventLoop() {
    while (_running) {
        int n = epoll_wait(epoll_fd, events, 64, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (server_by_fd.count(fd)) {
                handleNewConnection(fd);
            }
            else {
                handleClientRequest(fd);
            }
        }
    }
}

void Server::shutdown() {
    _running = false;
}

void Server::start() {
    initAllSockets();
    registerSocketsInEpoll();
    eventLoop();
}
