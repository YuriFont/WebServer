#include "../../include/handlers/MethodNotAllowedHandler.hpp"

MethodNotAllowedHandler::MethodNotAllowedHandler(const std::vector<std::string>& alloweds) {

    for (size_t i = 0; i < alloweds.size(); i++) {
        if (i > 0) 
            _methodsAllowed += ", ";
        _methodsAllowed += alloweds[i];
    }

};

MethodNotAllowedHandler::MethodNotAllowedHandler(const MethodNotAllowedHandler& other): _methodsAllowed(other._methodsAllowed), _response(NULL) {

};

MethodNotAllowedHandler::~MethodNotAllowedHandler() {
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
};

void MethodNotAllowedHandler::handleData(const std::string& chunk) {
    (void)chunk;
};

bool MethodNotAllowedHandler::isFinished() {
    return true;
};

HttpResponse& MethodNotAllowedHandler::getResponse() {
    _response = new HttpResponse();

    _response->setStatus(405);
    _response->setHeader("Allow", _methodsAllowed);
    _response->setConnectionClose(true);
    return *_response;
};
 
IMethodHandler* MethodNotAllowedHandler::clone() const {
    return new MethodNotAllowedHandler(*this);
};