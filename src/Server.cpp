#include "../include/WebServer.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include "../include/Config.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/RequestHandler.hpp"

Server::Server(const Config &config) : _config(config) {}
Server::~Server() {}

const Location &Server::findLocation(HttpRequest &request)
{
    std::string path = request.getPath();
    std::string match = "/";
    size_t longestMatch = 0;

    for (std::map<std::string, Location>::const_iterator it = _config.locations.begin();
         it != _config.locations.end(); ++it)
    {
        if (path.compare(0, it->first.size(), it->first) == 0 && it->first.size() > longestMatch)
        {
            longestMatch = it->first.size();
            match = it->first;
        }
    }

    std::map<std::string, Location>::const_iterator found = _config.locations.find(match);
    if (found == _config.locations.end())
        throw std::runtime_error("Nenhum location encontrado para path: " + path);
    return found->second;
}

void Server::initSocket()
{
    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_fd == -1)
    {
        std::cerr << "Erro no socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    fcntl(this->server_fd, F_SETFL, O_NONBLOCK);

    int opt = 1;
    if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Erro no setsockopt: " << strerror(errno) << std::endl;
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_config.port);
    addr.sin_addr.s_addr = inet_addr(_config.ip.c_str());

    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        std::cerr << "IP inválido: " << _config.ip << std::endl;
        exit(EXIT_FAILURE);
    }

    if (bind(this->server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Erro no bind: " << strerror(errno) << std::endl;
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }
}

void Server::startListenServer()
{
    if (listen(this->server_fd, 5) < 0)
    {
        std::cerr << "Erro no listen: " << strerror(errno) << std::endl;
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Servidor escutando na porta " << _config.port << "..." << std::endl;
}

void Server::createEpoll()
{
    this->epoll_fd = epoll_create(1);
    if (this->epoll_fd == -1)
    {
        std::cerr << "Erro ao criar epoll: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = this->server_fd;

    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->server_fd, &ev);
}

void Server::handleNewConnection()
{
    int client_fd = accept(this->server_fd, NULL, NULL);
    if (client_fd == -1)
        return;

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    std::cout << "Cliente conectado!" << std::endl;
}

void Server::handleClientRequest(int client_fd)
{
    char buffer[2048];
    int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes <= 0)
    {
        close(client_fd);
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        std::cout << "Cliente desconectado." << std::endl;
        return;
    }

    HttpRequest request(buffer);
    const Location &location = findLocation(request);

    if (!location.isMethodAllowed(request.getMethod()))
    {
        //TODO: implementar método não permitido 
        HttpResponse response = HttpResponse::methodNotAllowed(location.getMethods());
        send(client_fd, response.toString().c_str(), response.toString().size(), 0);
        return;
    }

    RequestHandler handler(_config);
    HttpResponse response = handler.handle(request, location);
    // (void)response;
    //TODO: enviar resposta ao cliente
    std::cout <<  response.toString() << std::endl;
    send(client_fd, response.toString().c_str(), response.toString().size(), 0);
}

void Server::eventLoop()
{
    while (true)
    {
        int n = epoll_wait(this->epoll_fd, this->events, 10, -1);
        for (int i = 0; i < n; i++)
        {
            int fd = this->events[i].data.fd;
            if (fd == this->server_fd)
                handleNewConnection();
            else
                handleClientRequest(fd);
        }
    }
}

void Server::start()
{
    initSocket();
    startListenServer();
    createEpoll();
    eventLoop();
}
