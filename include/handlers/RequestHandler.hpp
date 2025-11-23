#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Config.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "CgiHandler.hpp"

class RequestHandler {
public:
    RequestHandler(const Config &config);
    HttpResponse handle(HttpRequest &request, const Location &location);
    
private:
    const Config &_config;
};

#endif
