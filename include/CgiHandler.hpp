#pragma once

#include "WebServer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "Utils.hpp"

class CgiHandler{
    private:
        static std::vector<std::string> buildCgiEnv(const HttpRequest &request, const Location &location, const std::string &scriptPath);
        static void spawnCgiChild(const HttpRequest &request, const Location &location,
                           const std::string &path, const std::string &interpreter,
                           int inPipe[2], int outPipe[2]);
        static std::string readCgiOutput(int outPipe[2], pid_t pid);
        static HttpResponse responseHTTP(const std::string &output, HttpResponse &response);
    public:
        static HttpResponse process(const HttpRequest &request, const Location &location, std::string extension);
};