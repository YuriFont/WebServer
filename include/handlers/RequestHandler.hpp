#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../core/ServerConfig.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../handlers/GetHandler.hpp"
#include "../handlers/PostHandler.hpp"
#include "../handlers/DeleteHandler.hpp"
#include "../http/HttpResponse.hpp"
#include "../handlers/CgiHandler.hpp"
#include "../handlers/RedirectHandler.hpp"
#include "../handlers/NotImplementedHandler.hpp"
#include "../handlers/MethodNotAllowedHandler.hpp"
#include "CgiHandler.hpp"
#include "../interfaces/IMethodHandler.hpp"

class RequestHandler {
public:
    RequestHandler(const ServerConfig &config);
    static IMethodHandler* handle(const ServerConfig &server, HttpRequest &request, const Location &location, int client_fd);
    const ServerConfig& getConfig() {
        return this->_config;
    }
private:
    static bool isCgiEnabledForExtension(HttpRequest &request, const Location &location);
    const ServerConfig &_config;
};

#endif
