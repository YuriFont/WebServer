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
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, server_fds[i], NULL);
        close(server_fds[i]);
    }

    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
        close(it->first);
    }

    for (std::map<int, CgiProcess *>::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ++it) {

        CgiProcess* cgi = it->second;
        if (!cgi->stdin_closed) {
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, cgi->stdin_fd, NULL);
            close(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, cgi->stdout_fd, NULL);
            close(cgi->stdout_fd);
        }
        delete cgi;
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
    std::cout << GREEN << "New client in the server " << server_by_fd[server_fd].getPort() << RESET << std::endl;
    logClienteConected(client_fd);
}

const Location &Server::findLocation(ServerConfig *serverCfg, HttpRequest &request) {
    const std::map<std::string, Location> &locs = serverCfg->getLocations();
    std::string path = request.getPath();
    std::string match = "/";
    size_t longestMatch = 0;

    for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
        const std::string &locPath = it->first;

        if (path.compare(0, locPath.size(), locPath) != 0)
            continue;

        if (path.size() > locPath.size() && locPath != "/" && path[locPath.size()] != '/')
            continue;

        if (locPath.size() > longestMatch) {
            longestMatch = locPath.size();
            match = locPath;
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

    // tratar buffer
    if (bytes > 0) {
        return ;
    }
    else {
        //erro de leitura no socket
        removeClient(client_fd);
        logClientDesconected(client_fd);
    }
}

IMethodHandler* Server::buildMethodHandler(Client& client , int &client_fd) {

    HttpRequest& request = client.getRequest();
    const Location &location = findLocation(client_server[client_fd], request);
    const ServerConfig& serverConfig = *client_server[client_fd];
    return RequestHandler::handle(serverConfig, request, location, client_fd);
}

bool Server::removeMethodHandler(Client& client) {
    
    if (client.handler) {
        delete client.handler;
        client.handler = NULL;
    }

    return true;
}

void Server::finalizeClientConnection(const int &client_fd, Client& client, const bool& closeConnection) {

    if (closeConnection) {
        removeClient(client_fd);
    } else {
        client.cleanData();
    }
}

void Server::logClienteRequest(const int &client_fd, Client& client) {

    std::cout << "[FD " << client_fd << "] <--- Request: " 
            << client.getRequest().getMethod() << " " 
            << client.getRequest().getPath() << " "
            << client.getRequest().getHttpVersion()
            << std::endl;
}

void Server::logClienteConected(const int &client_fd) {
    std::cout << "[FD " << client_fd << "] (+) Connection established" << std::endl;
}

void Server::logClientDesconected(const int &client_fd) {
    std::cout << RED << "[FD " << client_fd << "] (-) Connection closed" << RESET << std::endl;
}

void Server::logStatusResponse(const int &client_fd, Client& client) {
    std::cout << "[FD " << client_fd << "]" << " ---> " << client.getRequest().getMethod() << " " << client.getRequest().getPath() << " (" << client.getCodeResponseStatus() <<  ") - "<< client.getLenBody() << " bytes" << std::endl;
}

void Server::prepareResponse(const int &client_fd, Client& client) {
    
    HttpResponse& resp = client.handler->getResponse();
    client.setResponse(resp.toString());
    client.setCloseConnection(resp.isConnectionClose());
    client.setCodeResponseStatus(resp.getStatusResponse());

    epoll_event& event = client.getDataEvent();
    event.events = EPOLLOUT;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
    // finalizeClientConnection(client_fd, client, closeConnection);
}

void Server::addBuffer(Client& client, char* buffer, int& bytes) {

    if (!client.isAllHeaders()) {
        client.addBuffer(std::string(buffer, bytes));
        if (client.isAllHeaders()) {
            logClienteRequest(client.getClienteFd(), client);
        }
    } else {
        client.addBody(std::string(buffer, bytes));
    }
}

void Server::epoll_add(int fd, uint32_t events) {
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));

    ev.events  = events;
    ev.data.fd = fd;

    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
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
    if (client.handler == NULL)
        client.handler = buildMethodHandler(client, client_fd);
    client.handler->handleData(client.getRequest().getBody());
    client.eraseBody();
    if (client.handler->isFinished()) {
        if (client.handler->isCgi()) {
            startCgiForClient(client);
        }
        else
            prepareResponse(client_fd, client);
    }
}

void Server::sendResponseClient(int client_fd) {

    Client& client = clients[client_fd];
    ssize_t bytesSend = send(client_fd, (client.getResponse().c_str() + client.getBytesSend()),  (client.getLenBody() - client.getBytesSend()), 0);

    // std::cout << "Bytes send: " << bytesSend << std::endl;
    if (bytesSend == -1) {
        std::cout << "[FD " << client_fd << "] Error while send the response" << std::endl;
        removeClient(client_fd);
        return;
    }


    client.addBytesSend(bytesSend);
    // Se precisa fazer cast e porque o tipo está errado
    if ((size_t)client.getBytesSend() == client.getLenBody()) {
        logStatusResponse(client_fd, client);
        epoll_event& event = client.getDataEvent();
        event.events = EPOLLIN;
        epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client_fd, &event);

        removeMethodHandler(client);
        finalizeClientConnection(client_fd, client, client.getCloseConnection());
    }
};

void Server::handleCgiWrite(int fd) {
    CgiProcess* cgi = _cgiByFd[fd];

    ssize_t n = write(fd, cgi->input.c_str(), cgi->input.size());
    if (n > 0) {
        cgi->input.erase(0, n);
        if (cgi->input.empty()) {
            if (!cgi->stdin_closed) {
                epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, 0);
                close(fd);
                cgi->stdin_closed = true;
                _cgiByFd.erase(cgi->stdin_fd);

            }
        }
    }
}


void Server::startCgiForClient(Client& client) {
    
    CgiHandler* cgiHandler = static_cast<CgiHandler*>(client.handler);

    CgiProcess* cgi = cgiHandler->startCgi();
    
    if (_cgiByFd.find(cgi->stdin_fd) != _cgiByFd.end() || _cgiByFd.find(cgi->stdout_fd) != _cgiByFd.end()) {
        std::cout << "Pipe já registrado =================================" << std::endl;
    }
    _cgiByFd[cgi->stdin_fd]  = cgi;
    _cgiByFd[cgi->stdout_fd] = cgi;
    
    epoll_add(cgi->stdout_fd, EPOLLIN);
    
    if (!cgi->input.empty()) {
        epoll_add(cgi->stdin_fd, EPOLLOUT);
    }
    else {
        close(cgi->stdin_fd);
        cgi->stdin_closed = true;
        _cgiByFd.erase(cgi->stdin_fd);
    }
}

void Server::removeCgiFd(const int& fd) {

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
    _cgiByFd.erase(fd);
}



void Server::finalizeCgiResponse(CgiProcess* cgi) {

    if (clients.find(cgi->client_fd) == clients.end()) {
        std::cerr << "Erro: CGI finalizou, mas o cliente (fd " << cgi->client_fd << ") já desconectou." << std::endl;
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        delete cgi;
        return;
    }

    Client& client = clients[cgi->client_fd];

    if (client.handler == NULL) {
        std::cerr << "Erro: Cliente existe, mas o handler é NULL." << std::endl;
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdout_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        delete cgi;
        return;
    }

    CgiHandler* cgiHandler = static_cast<CgiHandler*>(client.handler);

    HttpResponse response = cgiHandler->responseHTTP(cgi->output);


    client.setCloseConnection(response.isConnectionClose());
    client.setCodeResponseStatus(response.getStatusResponse());

    client.setResponse(response.toString());


    epoll_event& event = client.getDataEvent();
    event.events = EPOLLOUT; 
    // Modificar o cliente para poder escrever no socket
    epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client.getClienteFd(), &event);

    // 3️⃣ Cleanup
    if (!cgi->stdin_closed) {
        cgi->stdin_closed = true;
        removeCgiFd(cgi->stdin_fd);
    }
    if (!cgi->stdout_closed) {
        cgi->stdout_closed = true;
        removeCgiFd(cgi->stdout_fd);
    }
    delete cgi;
}

void Server::handleCgiRead(int fd) {
    CgiProcess* cgi = _cgiByFd[fd];
    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer));

    // std::cout << "Acabei de ler isso: " << buffer << std::endl;
    if (n > 0) {
        cgi->output.append(buffer, n);
        // std::cout << "Acabei de ler isso: \n" << cgi->output << std::endl;

    }
    else if (n == 0) {
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdout_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        waitpid(cgi->pid, NULL, 0);
        finalizeCgiResponse(cgi);
    } else {
        
        std::cout << "Acho que deu erro" << std::endl;
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdout_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        //remover cliente também ?
        delete cgi;
    }
}

void Server::handleCgiEvent(int fd, uint32_t ev) {
    CgiProcess* cgi = _cgiByFd[fd];

    if ((ev & EPOLLOUT) && fd == cgi->stdin_fd) {
        handleCgiWrite(fd);
    }
    
    if ((ev & (EPOLLIN | EPOLLHUP)) && fd == cgi->stdout_fd) {
        handleCgiRead(fd);
    }
}

void Server::eventLoop() {
    while (_running) {
        int n = epoll_wait(epoll_fd, events, 64, 1000);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (server_by_fd.count(fd)) {
                handleNewConnection(fd);
            } else if (_cgiByFd.count(fd)) {
                handleCgiEvent(fd, events[i].events);
            }
            else {
                if (events[i].events & EPOLLIN) {
                    handleClientRequest(fd);
                }
                if (events[i].events & EPOLLOUT) {
                    sendResponseClient(fd);
                }
            }
        }
    }
}

void Server::shutdown() {
    _running = false;
}

void signalHandler(int signum) {
    (void)signum;
    waitpid(-1, NULL, WNOHANG);
}

void Server::start() {
    signal(SIGCHLD, signalHandler);
    initAllSockets();
    registerSocketsInEpoll();
    eventLoop();
}
