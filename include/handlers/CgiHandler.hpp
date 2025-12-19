#pragma once

#include "../core/WebServer.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../utils/Utils.hpp"
#include "../interfaces/IMethodHandler.hpp"
#include "../abstracts/ABodyProcessor.hpp"

class CgiHandler : public IMethodHandler {

public:
    CgiHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
    CgiHandler(const CgiHandler& other);
    ~CgiHandler();

    IMethodHandler* clone() const;

    void handleData(const std::string& chunk);
    bool isFinished();
    HttpResponse& getResponse();

private:
    std::string getExtensionCgi();
    std::vector<std::string> buildCgiEnv(const HttpRequest& request, const Location& location,
                                         const std::string& scriptPath, size_t contentLen);

    void spawnCgiChild(const HttpRequest& request, const Location& location,
                       const std::string& path, const std::string& interpreter,
                       int inPipe[2], int outPipe[2], size_t contentLen);

    std::string readCgiOutput(int outPipe[2], pid_t pid);
    HttpResponse& responseHTTP(const std::string& output);

    HttpResponse& process(); // usa _body / request.bodyTempPath

    size_t getBodySizeFromTempFile(const std::string& path) const;
    std::string readTempFileToString(const std::string& path) const;

private:
    const ServerConfig& _config;
    const HttpRequest&  _request;
    const Location&     _location;

    HttpResponse* _response;
    bool _isFinish;

    std::string _body;

    ABodyProcessor* _bodyProcessor; // ✅ agora CGI também usa BodyProcessor
};
