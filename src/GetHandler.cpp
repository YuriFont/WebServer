#include "../include/GetHandler.hpp"
#include "../include/Utils.hpp"

HttpResponse GetHandler::isDir(const std::string& path, const HttpRequest &request, const Location &location, struct stat info, HttpResponse response){
    std::string indexPath = path +"/index.html";
    //Se houver index.html -> 200 OK
    if (stat(indexPath.c_str(), &info) == 0){
        std::string body;
        Utils::readFile(indexPath, body);
        response.setStatus(200);
        response.setContentType(Utils::getContentType(indexPath));
        response.setBody(body);
        return response;
    }
    //Se autoindex estiver ativo -> 200 OK (gerar listagem)
    if (location.isAutoindex()){
        std::string body = Utils::generateAutoindex(path, request.getPath());
        response.setStatus(200);
        response.setContentType("text/html");
        response.setBody(body);
        return response;
    }
    //Arquivo forbidden
    response.setStatus(403);
    response.setContentType("text/html");
    return response;
}

HttpResponse GetHandler::process(HttpRequest &request, const Location &location){
    HttpResponse response;
    struct stat info;

    //Construir o caminho do arquivo
    std::string path = Utils::buildPathRequisition(location.getPath(), location.getRoot(), request.getPath());

    //Configurar header básicos da resposta
    response.setHttpVersion(request.getHttpVersion());
    response.setConnectionClose(true);

    //Verificar se o arquivo/dir existe
    if (stat(path.c_str(), &info) != 0){
        //Arquivo não encontrado
        response.setStatus(404);
        response.setContentType("text/html");
        return response;
    }
    //Se for diretório
    if(S_ISDIR(info.st_mode)){
        response = GetHandler::isDir(path, request, location, info, response);
        return response;
    }
    //Se for um arquivo normal
    std::string body;
    if (!Utils::readFile(path, body)){
        response.setStatus(500); //Arquivo sem permissão de leitura
        response.setContentType("text/html");
        return response;
    }
    response.setStatus(200);
    response.setContentType(Utils::getContentType(path));
    response.setBody(body);
    return response;
}
