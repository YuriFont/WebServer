#include "../include/RequestHandler.hpp"
#include "../include/GetHandler.hpp"
#include "../include/PostHandler.hpp"
#include "../include/DeleteHandler.hpp"

RequestHandler::RequestHandler(const Config &config) : _config(config) {}

HttpResponse RequestHandler::handle(HttpRequest &request, const Location &location)
{
    std::string method = request.getMethod();

    if (method == "GET")
        return GetHandler::process(request, location);

    if (method == "POST")
        return PostHandler::process(request, location);

    if (method == "DELETE")
        return DeleteHandler::process(request, location);

    throw std::runtime_error("Unsupported HTTP method: " + method);
}
