#include "../../include/handlers/PostHandler.hpp"
#include "../../include/utils/Utils.hpp"
#include "../../include/utils/ErrorPage.hpp"

std::map<std::string, std::string> PostHandler::_types;


PostHandler::PostHandler(const ServerConfig& config, const HttpRequest& request, const Location& location): _config(config), _request(request), _location(location), _response(NULL), _bodyProcessor(NULL), _isFinish(false) {

};

PostHandler::PostHandler(const PostHandler& other): _config(other._config), _request(other._request), _location(other._location), _response(NULL), _isFinish(other._isFinish) {

    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};

PostHandler::~PostHandler() {
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
    if (_bodyProcessor != NULL) {
        delete _bodyProcessor;
        _bodyProcessor = NULL;
    }
};

void PostHandler::handleData(const std::string& chunk) {

    if (_request.isChunked()) {
        if (chunk.size() > _config.client_max_body_size) {
            _response = new HttpResponse();
            _response->setStatus(413);
            _response->setBody(ErrorPage::build(413));
            _isFinish = true;
            setIsError(true);
            return;
        }
    }
    if (_bodyProcessor == NULL) {
        _bodyProcessor = BodyProcessorFactory::createBodyProcessor(_config, _location, _request);
    }
    _bodyProcessor->handleChunk(chunk);
    if (_bodyProcessor->isFinished()) {
        _response = _bodyProcessor->getResult();
        _isFinish = true;
    }
};

bool PostHandler::isFinished() {
    return (_isFinish);
};

HttpResponse& PostHandler::getResponse() {

    HttpResponse& response = *_response;
    return response;

};

IMethodHandler* PostHandler::clone() const {
    return new PostHandler(*this);
};