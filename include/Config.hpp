#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "WebServer.hpp"
#include "Location.hpp"
#include "ServerConfig.hpp"

class Config {
    public:
        std::vector<ServerConfig> servers;
        Config(const std::string &filePath);
        ~Config();

    private:
        std::string _filePath;
        std::ifstream _file;
        void _parseConfigFile();
        void _openFile();
        void _skipComments(std::string &line);
        std::istream& _getUtilLine(std::istream &is, std::string &line);
        void _parseListen(std::istringstream &iss, ServerConfig &server);
        void _parseServerName(std::istringstream &iss, ServerConfig &server);
        void _parseErrorPages(std::istringstream &iss, ServerConfig &server);
        void _parseClientMaxBodySize(std::istringstream &iss, ServerConfig &server);
        void _parseLocation(std::istringstream &iss, ServerConfig &server);
        bool _IPValidation(const std::string &addressIP);
        bool _PortValidation(int addressPort);
};

#endif