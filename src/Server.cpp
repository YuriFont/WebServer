#include "../include/WebServer.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include "../include/Config.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/RequestHandler.hpp"

Server::Server(const Config &config) : _config(config) {}
Server::~Server() {}

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

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    client_server[client_fd] = &server_by_fd[server_fd];

    std::cout << "Novo cliente no server " << server_by_fd[server_fd].getPort() << std::endl;
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

void Server::handleClientRequest(int client_fd) {
    ServerConfig *serverCfg = client_server[client_fd];

    if (!serverCfg) {
        std::cerr << "Erro: client sem server associado" << std::endl;
        close(client_fd);
        return;
    }

    char buffer[2048];
    int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes <= 0) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        return;
    }

    buffer[bytes] = 0;
    std::string rawRequest(buffer);

    HttpRequest request(rawRequest.c_str());
    const Location &location = findLocation(serverCfg, request);

    RequestHandler handler(_config);
    HttpResponse resp = handler.handle(request, location);

    send(client_fd, resp.toString().c_str(), resp.toString().size(), 0);
}

void Server::eventLoop() {
    while (true) {
        int n = epoll_wait(epoll_fd, events, 64, -1);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            // Caso seja um dos sockets de servers (listener)
            if (server_by_fd.count(fd)) {
                handleNewConnection(fd);
            }
            else {
                handleClientRequest(fd);
            }
        }
    }
}

void Server::start() {
    initAllSockets();
    registerSocketsInEpoll();
    eventLoop();
}
