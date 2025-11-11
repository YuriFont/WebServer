#include "../include/CgiHandler.hpp"

HttpResponse CgiHandler::process(const HttpRequest &request, const Location &location)
{
    (void)location;
    std::cout << "[DEBUG] Entrou em CgiHandler::process()" << std::endl;

    HttpResponse response;
    response.setHttpVersion(request.getHttpVersion());
    response.setStatus(200);
    response.setContentType("text/html");
    response.setConnectionClose(true);
    return response;
}