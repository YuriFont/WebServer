#include "../../include/handlers/RequestHandler.hpp"

RequestHandler::RequestHandler(const ServerConfig &config) : _config(config) {}

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

IMethodHandler* RequestHandler::handle(const ServerConfig &config, HttpRequest &request, const Location &location)
{
    std::string method = request.getMethod();
    std::string requestExt = Utils::getExtension(request.getPath());

    if ((location.isCgiEnabled() && isCgiEnabledForExtension(request, location)) || (config.hasGlobalCGI && config.hasExtGlobalCgi(requestExt)))
        return new CgiHandler(config, request, location);

    if (!location.isMethodAllowed(method)) {
        return new MethodNotAllowedHandler(location.getMethods());
    }

    if (!location.getRedirect().empty()) {
        return new RedirectHandler(config, request, location);
    }

    if (method == "GET")
        return new GetHandler(config, request, location);

    if (method == "POST")
        return new PostHandler(config, request, location);

    if (method == "DELETE")
        return new DeleteHandler(config, request, location);

    return new NotImplementedHandler(config, request, location);
}
