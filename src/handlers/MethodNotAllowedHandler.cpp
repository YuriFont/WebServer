#include "../../include/handlers/MethodNotAllowedHandler.hpp"

MethodNotAllowedHandler::MethodNotAllowedHandler(const std::vector<std::string>& alloweds) {

    for (size_t i = 0; i < alloweds.size(); i++) {
        if (i > 0) 
            _methodsAllowed += ", ";
        _methodsAllowed += alloweds[i];
    }

};

MethodNotAllowedHandler::MethodNotAllowedHandler(const MethodNotAllowedHandler& other): _methodsAllowed(other._methodsAllowed) {

};

MethodNotAllowedHandler::~MethodNotAllowedHandler() {

};

void MethodNotAllowedHandler::handleData(const std::string& chunk) {
    (void)chunk;
};

bool MethodNotAllowedHandler::isFinished() {
    return true;
};

HttpResponse& MethodNotAllowedHandler::getResponse() {
    HttpResponse *response = new HttpResponse();

    response->setStatus(405);
    response->setHeader("Allow", _methodsAllowed);
    return *response; 
};
 
IMethodHandler* MethodNotAllowedHandler::clone() const {
    return new MethodNotAllowedHandler(*this);
};