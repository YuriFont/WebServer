#include "../../include/handlers/DeleteHandler.hpp"
#include "../../include/utils/Utils.hpp"

DeleteHandler::DeleteHandler(const Config& config, const HttpRequest& request, const Location& location):  _config(config), _request(request), _location(location), _response(NULL), _isFinish(false) {

};
DeleteHandler::DeleteHandler(const DeleteHandler &other): _config(other._config), _request(other._request), _location(other._location), _response(NULL), _isFinish(other._isFinish) {
    
    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};

DeleteHandler::~DeleteHandler() {

    if (_response != NULL)
        delete _response;
};
void DeleteHandler::handleData(const std::string& chunk) {
    (void)chunk;
    this->_isFinish = true;
};

bool DeleteHandler::isFinished() {

    return (_isFinish);
};

HttpResponse& DeleteHandler::getResponse() {
    
    return process(_request, _location);
};

IMethodHandler* DeleteHandler::clone() const {
    return new DeleteHandler(*this);
};

HttpResponse& DeleteHandler::process(const HttpRequest &request, const Location &location)
{
    // Implementação do DELETE ficará aqui
    std::string path = Utils::buildPathRequisition(location.getPath(), location.getRoot(), request.getPath());

    _response = new HttpResponse();
    struct stat info;

    _response->setHttpVersion(request.getHttpVersion());
    _response->setContentLength(0);
    _response->setConnectionClose(true);

    if (stat(path.c_str(), &info) != 0) {

        if (errno == EACCES)
            _response->setStatus(403);
        else if (errno == ENOENT)
            _response->setStatus(404);
        else
            _response->setStatus(500);
    } else if (S_ISDIR(info.st_mode)) {
        // retornar not allow 403, diretorio ou permissao
        _response->setStatus(403);
    } else if (remove(path.c_str()) == 0) {
        // se der error ao remover mandar 500;
        _response->setStatus(204);
    } else {
       if (errno == EACCES || errno == EPERM)
            _response->setStatus(403);
        else
            _response->setStatus(500);
    }

    return (*_response);
}
