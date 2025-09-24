#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "WebServer.hpp"

class Config {
    public:
        Config(const std::string &filePath);
        ~Config();
    private:
        std::string filePath;
        void parseConfigFile();
};

#endif