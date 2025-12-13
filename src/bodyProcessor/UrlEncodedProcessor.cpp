#include "../../include/bodyProcessor/UrlEncodedProcessor.hpp"


UrlEncodedProcessor::UrlEncodedProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request) : ABodyProcessor::ABodyProcessor(config), _location(location), _bytesReceived(0), _response(NULL) {

    _contentType = request.getHeader("Content-Type");
    _contentLength = request.getContentLength();
};

bool UrlEncodedProcessor::isMaxBodySize() {
    if (_contentLength > _config.client_max_body_size)
        return true;
    else if (_bytesReceived > _config.client_max_body_size)
        return true;
    return false;
};

UrlEncodedProcessor::~UrlEncodedProcessor() {
    if (_response != NULL)
        delete _response;
};

void UrlEncodedProcessor::handleChunk(const std::string& chunk) {

    if (isMaxBodySize()) {
        _response = new HttpResponse();
        _isFinished = true;
        _response->setStatus(413);
        return;
    }
    _body.append(chunk);
    if (_body.size() >= _contentLength) {
        _response = new HttpResponse();
        handleFormUrlencoded(_body);
        _isFinished = true;
    }
};


bool UrlEncodedProcessor::isFinished() {
    return _isFinished;
};


HttpResponse* UrlEncodedProcessor::getResult() {
    return new HttpResponse(*_response);
};


void UrlEncodedProcessor::handleFormUrlencoded(const std::string& body) {

    std::string uploadPath = _location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".txt";
    if (Utils::writeFile(uploadPath, body)) {
        _response->setStatus(201);
        _response->setContentType("text/html");
        _response->setBody("File uploaded successfully to " + uploadPath);
    } else {
        _response->setStatus(500);
        _response->setContentType("text/html");
        _response->setBody("500 Internal Server Error: Failed to save the uploaded file.");
    }
}