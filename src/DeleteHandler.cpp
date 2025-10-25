#include "../include/DeleteHandler.hpp"
#include "../include/Utils.hpp"

HttpResponse DeleteHandler::process(HttpRequest &request, const Location &location)
{
    // Implementação do DELETE ficará aqui
    std::string path = Utils::buildPathRequisition(location.getPath(), location.getRoot(), request.getPath());

    HttpResponse response;
    struct stat info;

    response.setHttpVersion(request.getHttpVersion());
    response.setContentLength(0);
    response.setConnectionClose(true);

    if (stat(path.c_str(), &info) != 0) {
        // retornar 404, não existe
        response.setStatus(404);
    } else if (S_ISDIR(info.st_mode)) {
        // retornar not allow 403, diretorio ou permissao
        response.setStatus(403);
    } else if (remove(path.c_str()) == 0) {
        // se der error ao remover mandar 500;
        response.setStatus(204);
    } else {
        response.setStatus(500);
    }

    return (response);
}
