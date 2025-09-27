#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "WebServer.hpp"

class Config {
    public:
        std::string server_name;
        std::string upload_path;
        std::string cgi_path;
        std::string cgi_extension;
        std::string ip;
        int port;
        std::map<int, std::string> error_page;
        size_t client_max_body_size;
        bool autoindex;
        Config(const std::string &filePath);
        ~Config();
    private:
        std::string _filePath;
        std::ifstream _file;
        void _parseConfigFile();
        void _openFile();
        void _skipComments(std::string &line);
        std::istream& _getUtilLine(std::istream &is, std::string &line);
        void Config::_parseListen(std::istringstream &iss, std::string &line);
        //void Config::_parseServerName(std::istringstream &iss, std::string &line);
        //void Config::_parseErrorPage(std::istringstream &iss, std::string &line);
        //void Config::_parseClientMaxBodySize(std::istringstream &iss, std::string &line);
        //void Config::_parseLocation(std::istringstream &iss, std::string &line);
};

#endif