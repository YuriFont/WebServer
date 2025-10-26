#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "WebServer.hpp"
#include "Location.hpp"

class Config {
    public:
        std::string server_name;
        std::string ip;
        int port;
        std::map<int, std::string> error_page;
        size_t client_max_body_size;
        std::map<std::string, Location> locations;
        Config(const std::string &filePath);
        ~Config();

    private:
        std::string _filePath;
        std::ifstream _file;
        void _parseConfigFile();
        void _openFile();
        void _skipComments(std::string &line);
        std::istream& _getUtilLine(std::istream &is, std::string &line);
        void _parseListen(std::istringstream &iss);
        void _parseServerName(std::istringstream &iss);
        void _parseErrorPages(std::istringstream &iss);
        void _parseClientMaxBodySize(std::istringstream &iss);
        void _parseLocation(std::istringstream &iss);
        bool _IPValidation(const std::string &addressIP);
        bool _PortValidation(int addressPort);
};

#endif