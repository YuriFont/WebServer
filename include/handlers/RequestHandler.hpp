#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../core/ServerConfig.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "CgiHandler.hpp"
#include "../interfaces/IMethodHandler.hpp"

class RequestHandler {
public:
    RequestHandler(const ServerConfig &config);
    static IMethodHandler* handle(const ServerConfig &server, HttpRequest &request, const Location &location);
    
private:

    static bool isCgiEnabledForExtension(HttpRequest &request, const Location &location);
    const ServerConfig &_config;
};

#endif
