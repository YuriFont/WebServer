#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"

class DeleteHandler {
public:
    static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
