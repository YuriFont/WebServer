#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../config/Config.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "CgiHandler.hpp"

class RequestHandler {
public:
    RequestHandler(const Config &config);
    HttpResponse handle(HttpRequest &request, const Location &location, const ServerConfig &server);
    
private:
    const Config &_config;
};

#endif
