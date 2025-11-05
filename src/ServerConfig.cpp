#include "../include/ServerConfig.hpp"

ServerConfig::ServerConfig(): ip("127.0.0.1"), port(8080), socket_fd(-1){}

ServerConfig::ServerConfig(const std::string &ip, int port, const std::map<std::string, Location> &locs): ip(ip), port(port), locations(locs), socket_fd(-1) {}

ServerConfig::~ServerConfig() {
}

ServerConfig::ServerConfig(const ServerConfig& server) {
    *this = server;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
    if (this != &other) {
        this->ip = other.ip;
        this->port = other.port;
        this->socked_fd = other.socked_fd;
        this->locations = other.locations;
    }
    return *this;
}

int ServerConfig::getSocket() const{
    return this->socket_fd;
}

const std::map<std::string, Location>& ServerConfig::getLocations() const{
    return this->locations;
}

std::string ServerConfig::getIp() const{
    return this->ip;
}

int ServerConfig::getPort() const{
    return this->port;
}

int initSocket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cerr << "Erro no socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Erro no setsockopt: " << strerror(errno) << std::endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        std::cerr << "IP inválido: " << _config.ip << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Erro no bind: " << strerror(errno) << std::endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    if (listen(socket_fd, 5) < 0)
    {
        std::cerr << "Erro no listen: " << strerror(errno) << std::endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Servidor ouvindo em " << ip << ":" << port << std::endl;
    return socket_fd;
}