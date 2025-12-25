#include "../../include/handlers/GetHandler.hpp"
#include "../../include/utils/Utils.hpp"
#include "../../include/utils/ErrorPage.hpp"


// GetHandler::GetHandler(): _respons {};

GetHandler::GetHandler(const GetHandler& other): _config(other._config), _request(other._request), _location(other._location), _response(NULL) {

    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};

// GetHandler& GetHandler::operator=(const GetHandler& other) {

//     if (this != &other) {
//     }
//     return (*this);
// };

GetHandler::~GetHandler() {

    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
};

GetHandler::GetHandler(const ServerConfig& config, const HttpRequest& request, const Location& location) : _config(config), _request(request), _location(location), _response(NULL) {
};


void GetHandler::handleData(const std::string& chunk) {
    (void)chunk;
};

bool GetHandler::isFinished() {

    return (true);
};

IMethodHandler* GetHandler::clone() const {
    return new GetHandler(*this);
};

HttpResponse& GetHandler::getResponse() {
    return process(_request, _location);
};

void GetHandler::isDir(const std::string& path, const HttpRequest &request, const Location &location, struct stat info, HttpResponse& response){

    const std::vector<std::string>& indexs = location.getIndex();
    
    
    for (size_t i = 0; i < indexs.size(); i++) {
        std::string indexPath = path[path.length() - 1] == '/' ? path + indexs[i] : path + "/" + indexs[i];
        // std::cout << "entrou no diretorio: " <<  indexPath << std::endl;
        if (stat(indexPath.c_str(), &info) == 0){
            // std::cout << "Achei o arquivo: " <<  indexPath << std::endl;
            std::string body;
            if (Utils::readFile(indexPath, body)) {
                response.setStatus(200);
                response.setContentType(Utils::getContentType(indexPath));
                response.setBody(body);
            } else {
                response.setStatus(500); // Internal Server Error
                response.setConnectionClose(true);
                response.setBody(ErrorPage::build(500));
            }
            return ;
        }
    }
    std::string indexPath = path +"/index.html";
    //Se houver index.html -> 200 OK
    if (stat(indexPath.c_str(), &info) == 0){
        std::string body;
        if (Utils::readFile(indexPath, body)) {
            response.setStatus(200);
            response.setContentType(Utils::getContentType(indexPath));
            response.setBody(body);
        }
        else {
            response.setStatus(500);
            response.setConnectionClose(true);
            response.setBody(ErrorPage::build(500));
        }
        return ;
    }
    //Se autoindex estiver ativo -> 200 OK (gerar listagem)
    if (location.isAutoindex()){
        std::string body = Utils::generateAutoindex(path, request.getPath());
        response.setStatus(200);
        response.setContentType("text/html");
        response.setBody(body);
        return ;
    }
    //Arquivo forbidden
    response.setStatus(404);
    response.setBody(ErrorPage::build(404));
    response.setContentType("text/html");
}

HttpResponse& GetHandler::process(const HttpRequest &request, const Location &location){

    _response = new HttpResponse();
    struct stat info;

    //Construir o caminho do arquivo
    std::string path = Utils::buildPathRequisition(location.getPath(), location.getRoot(), request.getPath());

    //Configurar header básicos da resposta
    _response->setHttpVersion(request.getHttpVersion());

    //Verificar se o arquivo/dir existe
    if (stat(path.c_str(), &info) != 0){
        //Arquivo não encontrado
        _response->setStatus(404);
        _response->setBody(ErrorPage::build(404));
        _response->setContentType("text/html");
        return *_response;
    }
    //Se for diretório
    if(S_ISDIR(info.st_mode)){
        isDir(path, request, location, info, *_response);
        return *_response;
    }
    //Se for um arquivo normal
    std::string body;
    if (!Utils::readFile(path, body)){
        _response->setStatus(500); //Arquivo sem permissão de leitura
        _response->setContentType("text/html");
        return *_response;
    }
    _response->setStatus(200);
    _response->setContentType(Utils::getContentType(path));
    _response->setBody(body);
    return *_response;
}
