#include "../../include/handlers/RedirectHandler.hpp"
#include "../../include/utils/Utils.hpp"


RedirectHandler::RedirectHandler(const ServerConfig& config, const HttpRequest& request, const Location& location): _config(config), _request(request), _location(location), _response(NULL), _isFinish(false) {

};

RedirectHandler::RedirectHandler(const RedirectHandler& other): _config(other._config), _request(other._request), _location(other._location), _response(NULL), _isFinish(other._isFinish) {

    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};

RedirectHandler::~RedirectHandler() {

    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
};

void RedirectHandler::handleData(const std::string& chunk) {

    if (_response == NULL) {
        _response = new HttpResponse();
        _response->setHttpVersion(_request.getHttpVersion());
        _response->setStatus(_location.getRedirectCode());
        _response->setHeader("Location", _location.getRedirect());
        _response->setContentLength(0);
        if (chunk.size() > 0)
            _response->setBody(chunk);
        _response->setConnectionClose(true);
    }
    _isFinish = true;
};

bool RedirectHandler::isFinished() {
    return (_isFinish);
};

HttpResponse& RedirectHandler::getResponse() {

    //    Redirecionamento global (antes de qualquer método)
    return *_response;
};

IMethodHandler* RedirectHandler::clone() const {
    return new RedirectHandler(*this);
};