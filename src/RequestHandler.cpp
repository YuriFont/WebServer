#include "../include/RequestHandler.hpp"
#include "../include/GetHandler.hpp"
#include "../include/PostHandler.hpp"
#include "../include/DeleteHandler.hpp"
#include "../include/HttpResponse.hpp"

RequestHandler::RequestHandler(const Config &config) : _config(config) {}

HttpResponse RequestHandler::handle(HttpRequest &request, const Location &location)
{
    //Redirecionamento global (antes de qualquer método)
    //resquícios de código legado, quando estava funcionando apenas um servidor
    (void)_config;
    if (!location.getRedirect().empty()){
        HttpResponse response;
        response.setHttpVersion(request.getHttpVersion());
        response.setStatus(location.getRedirectCode());
        response.setHeader("Location", location.getRedirect());
        response.setContentLength(0);
        response.setConnectionClose(true);
        return response;
    }
    std::string method = request.getMethod();

    if (method == "GET")
        return GetHandler::process(request, location);

    if (method == "POST")
        return PostHandler::process(request, location);

    if (method == "DELETE")
        return DeleteHandler::process(request, location);

    throw std::runtime_error("Unsupported HTTP method: " + method);
}
