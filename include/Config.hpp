#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "WebServer.hpp"

class Config {
    public:
        Config(const std::string &filePath);
        ~Config();
    private:
        std::string _filePath;
        std::ifstream _file;
        void _parseConfigFile();
        void _openFile();
        void _skipComments(std::string &line);
};

#endif