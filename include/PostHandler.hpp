#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"

class PostHandler {
public:
    static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
