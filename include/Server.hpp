#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServer.hpp"
#include "Config.hpp"

class Server {
    public:
        Server(const Config &config);
        ~Server();
        void start();

    private:
        const Config& _config;
};

#endif