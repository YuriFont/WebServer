#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"

class CgiHandler{
    public:
        static HttpResponse process(const HttpRequest &request, const Location &location);
};