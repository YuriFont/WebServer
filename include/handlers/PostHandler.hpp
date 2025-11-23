#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"

class PostHandler {
public:
    static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
