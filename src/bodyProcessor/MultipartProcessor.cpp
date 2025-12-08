#include "../../include/bodyProcessor/MultipartProcessor.hpp"


MultipartProcessor::MultipartProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request): ABodyProcessor::ABodyProcessor(config), _location(location), _bytesReceived(0), _response(NULL), _currentState(READING_HEADERS)  {

    _contentType = request.getHeader("Content-Type");
    _boundary = getBoundary(_contentType);
    _endBoundary = _boundary + "--";
    _contentLength = request.getContentLength();
};

MultipartProcessor::~MultipartProcessor() {

    if (_response != NULL)
        delete _response;
};

void MultipartProcessor::handleChunk(const std::string& chunk) {

    handleMultipart(chunk);
    if (_bytesReceived >= _contentLength) {
        _response = new HttpResponse();
        _response->setStatus(201);
        _response->setBody("<h2>Tudo certo</h2>");
        _isFinished = true;
    }
};

bool MultipartProcessor::isFinished() {
    return _isFinished;
};

HttpResponse* MultipartProcessor::getResult() {
    return new HttpResponse(*_response);
};

bool MultipartProcessor::append(std::string data, size_t len) {
    
    if (!_outFile.is_open() || !_outFile.good()) 
        return false;
    _outFile.write(data.c_str(), len);
    if (!_outFile) 
        return false;
    return true;
}

void MultipartProcessor::handleMultipart(const std::string& chunk) {


    _body.append(chunk);
    _bytesReceived += chunk.size();

    // mudar para aceitar também quando o chunck estiver completo
    if (_currentState == READING_HEADERS) {

        size_t endHeaders = _body.find("\r\n\r\n");
        if (endHeaders == std::string::npos)
            return ;
        _currentState = READING_BODY;
        std::string contents = _body.substr(0, endHeaders);
        _body.erase(0, endHeaders + 4);
        parseHeaderLine(contents, _currentContents);
        std::string uploadStore = _location.getUploadStore() + "/";
        std::string filePath =  _currentContents.fileName.empty() ? uploadStore + _currentContents.name + ".txt" : uploadStore + _currentContents.fileName;
        _outFile.open(filePath.c_str(), std::ios::binary | std::ios::app);
    }

    if (_currentState == READING_BODY) {

        size_t endBody = _body.find(_boundary);
        if (endBody == std::string::npos) {
            size_t boundarySize = _boundary.length();

            if (_body.size() > boundarySize) {
                size_t bytesToWrite = _body.size() - boundarySize;
                
                append(_body, bytesToWrite); 
                _body.erase(0, bytesToWrite);
            }
        } else {
            size_t bytesToWrite = endBody;
            if (bytesToWrite >= 2 && _body.substr(bytesToWrite - 2, 2) == "\r\n") {
                bytesToWrite -= 2;
            }

            append(_body, bytesToWrite); 

            _outFile.close(); 
            
            // verificar fim
            size_t endOfBoundary = endBody + _boundary.length();
            if (_body.substr(endOfBoundary, 2) == "--") {
                _currentState = READY; //está pronto
                _body.clear();
            } else {
                _currentState = READING_HEADERS; // ler o proximo header
                _currentContents.disposition.erase();
                _currentContents.name.erase();
                _currentContents.fileName.erase();
                _currentContents.contentType.erase();
                _body.erase(0, endOfBoundary + 2); // +2 para pular o \r\n depois da boundary, ler o proximo hearder
            }
        }
    }
};

// bool MultipartProcessor::append(std::string data, size_t len) {};

void MultipartProcessor::createFile() {};

void MultipartProcessor::finishFileUpload() {};

void MultipartProcessor::parseHeaderLine(const std::string& line, content& result) {
    // Caso 1: Content-Disposition
    if (line.find("Content-Disposition:") != std::string::npos) {
        size_t start = line.find(": ");
        size_t end = line.find(";");
        
        if (start != std::string::npos && end != std::string::npos) {
            // +2 para pular ": "
            result.disposition = line.substr(start + 2, end - (start + 2));
        }

        result.name = Utils::extractValue(line, "name=\"");
        result.fileName = Utils::extractValue(line, "filename=\"");
    }
    // Caso 2: Content-Type
    if (line.find("Content-Type:") != std::string::npos) {
        size_t start = line.find(": ");
        if (start != std::string::npos) {
            // Pega tudo do ": " até o fim da string (ou remove espaços extras se necessário)
            std::string type = line.substr(start + 2);
            
            // Opcional: Remover \r ou \n se a string vier bruta do socket
            size_t endClean = type.find_last_not_of(" \t\r\n");
            if (endClean != std::string::npos)
                 result.contentType = type.substr(0, endClean + 1);
            else 
                 result.contentType = type;
        }
    }
}

std::string MultipartProcessor::getBoundary(const std::string& contentType) {

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        std::cout << "Errou" << std::endl;
    }
    pos += 9;
    std::string boundary = contentType.substr(pos);
    boundary =  "--" + boundary;

    return boundary;
}