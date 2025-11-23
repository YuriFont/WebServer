#include "../include/RequestHandler.hpp"
#include "../include/GetHandler.hpp"
#include "../include/PostHandler.hpp"
#include "../include/DeleteHandler.hpp"
#include "../include/HttpResponse.hpp"
#include "../include/CgiHandler.hpp"

RequestHandler::RequestHandler(const Config &config) : _config(config) {}

HttpResponse RequestHandler::handle(HttpRequest &request, const Location &location)
{
    //Redirecionamento global (antes de qualquer método)
    if (!location.getRedirect().empty()){
        HttpResponse response;
        response.setHttpVersion(request.getHttpVersion());
        response.setStatus(location.getRedirectCode());
        response.setHeader("Location", location.getRedirect());
        response.setContentLength(0);
        response.setConnectionClose(true);
        return response;
    }

    // Detecção de CGI
    std::string rawPath = request.getPath();

    // Remover query string
    size_t q = rawPath.find('?');
    std::string path = (q != std::string::npos) ? rawPath.substr(0, q) : rawPath;

    // Agora extrair extensão corretamente
    size_t dot = path.find_last_of('.');

    if (dot != std::string::npos) {
        std::string extension = path.substr(dot);  // → ".py"
        if (location.hasCgiForExtension(extension))
            return CgiHandler::process(request, location, extension);
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
