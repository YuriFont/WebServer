#include "../../include/handlers/RequestHandler.hpp"
#include "../../include/handlers/GetHandler.hpp"
#include "../../include/handlers/PostHandler.hpp"
#include "../../include/handlers/DeleteHandler.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/handlers/CgiHandler.hpp"
#include "../../include/handlers/RedirectHandler.hpp"

RequestHandler::RequestHandler(const Config &config) : _config(config) {}

bool RequestHandler::isCgiEnabledForExtension(HttpRequest &request, const Location &location) {

        // Detecção de CGI
    std::string rawPath = request.getPath();

    // Remover query string
    size_t q = rawPath.find('?');
    std::string path = (q != std::string::npos) ? rawPath.substr(0, q) : rawPath;

    // Agora extrair extensão corretamente
    size_t dot = path.find_last_of('.');

    if (dot == std::string::npos) {
        return false;
    }
    std::string extension = path.substr(dot);  // → ".py"
    if (location.hasCgiForExtension(extension))
        return true;
    return false;
}

IMethodHandler* RequestHandler::handle(const Config &config, HttpRequest &request, const Location &location)
{

    if (!location.getRedirect().empty()) {
        return new RedirectHandler(config, request, location);
    }
    if (location.isCgiEnabled() && isCgiEnabledForExtension(request, location))
        return new CgiHandler(config, request, location);

    std::string method = request.getMethod();
    if (method == "GET")
        return new GetHandler(config, request, location);

    // if (method == "POST")
    //     return PostHandler::process(request, location);

    // if (method == "DELETE")
    //     return DeleteHandler::process(request, location);

    // throw std::runtime_error("Unsupported HTTP method: " + method);
    return NULL;
}
