#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"

class PostHandler {
public:
    static HttpResponse process(HttpRequest &request, const Location &location, const ServerConfig &server);
};

#endif
