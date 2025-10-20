#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"

class GetHandler {
public:
    static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
