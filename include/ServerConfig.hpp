#pragma once

#include "Location.hpp"

class ServerConfig {
    public:
        std::string server_name;
        std::string ip;
        int port;
        int socket_fd;
        std::map<int, std::string> error_pages;
        size_t client_max_body_size;
        std::map<std::string, Location> locations;

        ServerConfig();
        ServerConfig(const ServerConfig& server);
        ServerConfig& operator=(const ServerConfig& other);
        ~ServerConfig();

        int getSocket() const;
        const std::map<std::string, Location> &getLocations() const;
        std::string getIp() const;
        int getPort() const;
        int initSocket();
};