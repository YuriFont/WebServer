#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../config/Config.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "CgiHandler.hpp"
#include "../interfaces/IMethodHandler.hpp"

class RequestHandler {
public:
    RequestHandler(const Config &config);
    static IMethodHandler* handle(const Config &config, HttpRequest &request, const Location &location);
    
private:

    static bool isCgiEnabledForExtension(HttpRequest &request, const Location &location);
    const Config &_config;
};

#endif
