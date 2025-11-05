#pragma once

#include "Location.hpp"

class ServerConfig{
    private:
        std::string ip;
        int port;
        int socked_fd;
        std::map<std::string, Location> locations;
    public:
        ServerConfig();
        ServerConfig(const std::string &ip, int port, const std::map<std::string, Location> locs);
        ServerConfig(const ServerConfig& server);
        ServerConfig& operator=(const ServerConfig& other);
        ~ServerConfig();

        int getSocket() const;
        const std::map<std::string, Location> &getLocations() const;
        std::string getIp() const;
        int getPort() const;
        int initSocket();
};