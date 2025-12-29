#include "../../include/handlers/NotImplementedHandler.hpp"
#include "../../include/utils/Utils.hpp"

NotImplementedHandler::NotImplementedHandler(const ServerConfig& config, const HttpRequest& request, const Location& location): _config(config), _request(request), _location(location), _response(NULL), _isFinish(false) {};

NotImplementedHandler::NotImplementedHandler(const NotImplementedHandler& other): _config(other._config), _request(other._request), _location(other._location), _response(NULL), _isFinish(other._isFinish) {

    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};

NotImplementedHandler::~NotImplementedHandler() {
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
};
void NotImplementedHandler::handleData(const std::string& chunk) {
    
    (void)chunk;
    _response = new HttpResponse();
    _response->setStatus(501);
    std::string body = std::string("<!doctype html>") +
        "<html lang=\"pt-BR\">" +
        "    <head>" +
        "        <meta charset=\"utf-8\">"+
        "        <title>501 Not Implemented</title>"+
        "        <style>body{font-family:system-ui,Arial;margin:40px;color:#111}code{background:#f2f4f7;padding:2px 6px;border-radius:6px}</style>"+
        "    </head>"+
        "    <body>"+
        "        <h2>501 — Método não implementado</h2>"+
        "        <p>O método <code>{{METHOD}}</code> não é suportado por este servidor.</p>"+
        "        <p style=\"color:#666;font-size:13px\">Tente: <code>GET</code>, <code>POST</code> ou <code>DELETE</code>.</p>"+
        "    </body>"+
        "</html>";
    _response->setBody(body);
    _isFinish = true;
};

bool NotImplementedHandler::isFinished() {
    return (_isFinish);
};


HttpResponse& NotImplementedHandler::getResponse() {

    HttpResponse& response = *_response;
    return response;
};

IMethodHandler* NotImplementedHandler::clone() const {
    return new NotImplementedHandler(*this);
};