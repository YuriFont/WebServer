#pragma once

#include "../core/WebServer.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../utils/Utils.hpp"
#include "../interfaces/IMethodHandler.hpp"

class CgiHandler : public IMethodHandler {

    private:

        const ServerConfig& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        std::string _body;
        bool _isFinish;
        std::vector<std::string> buildCgiEnv(const HttpRequest &request, const Location &location, const std::string &scriptPath);
        void spawnCgiChild(const HttpRequest &request, const Location &location,
                           const std::string &path, const std::string &interpreter,
                           int inPipe[2], int outPipe[2]);
        std::string readCgiOutput(int outPipe[2], pid_t pid);
        HttpResponse& responseHTTP(const std::string &output);
        CgiHandler();
        CgiHandler& operator=(const CgiHandler& other);
    public:
        CgiHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
        CgiHandler(const CgiHandler& other);
        ~CgiHandler(); 
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
        std::string getExtensionCgi();
        HttpResponse& process(const HttpRequest &request, const Location &location, std::string extension);
};