#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"

class DeleteHandler {
    public:
        static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
