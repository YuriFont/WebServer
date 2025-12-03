#include "../../include/handlers/PostHandler.hpp"
#include "../../include/utils/Utils.hpp"

std::map<std::string, std::string> PostHandler::_types;

void PostHandler::initTypes() {

    if (!_types.empty())
        return;

    _types["image/jpeg"] = ".jpg";
    _types["image/png"]  = ".png";
    _types["image/gif"]  = ".gif";
    _types["image/webp"] = ".webp";
    _types["image/svg+xml"] = ".svg";
    _types["image/bmp"]  = ".bmp";
    _types["image/tiff"] = ".tiff";

    // Adicionando casos curtos se o seu parser enviar apenas o subtipo "jpeg" em vez de "image/jpeg"
    _types["jpeg"] = ".jpg";
    _types["png"]  = ".png";
    _types["gif"]  = ".gif";
    _types["webp"] = ".webp";
    _types["svg+xml"] = ".svg";
    _types["bmp"]  = ".bmp";
    _types["tiff"] = ".tiff";

    // --- AUDIO ---
    _types["audio/mpeg"] = ".mp3";
    _types["audio/wav"]  = ".wav";
    _types["audio/aac"]  = ".aac";
    
    // Casos curtos (se necessário)
    _types["mpeg"] = ".mp3";
    _types["wav"]  = ".wav";
    _types["aac"]  = ".aac";

    // --- VIDEO ---
    _types["video/mp4"]        = ".mp4";
    _types["video/x-matroska"] = ".mkv";
    _types["video/x-msvideo"]  = ".avi";
    
    // Casos curtos
    _types["mp4"]        = ".mp4";
    _types["x-matroska"] = ".mkv";
    _types["x-msvideo"]  = ".avi";

    // --- DOCUMENTOS / TEXTO ---
    _types["application/pdf"] = ".pdf";
    _types["text/plain"]      = ".txt";
    _types["text/csv"]        = ".csv";
    
    // Microsoft Office (Nomes longos)
    _types["application/msword"] = ".doc";
    _types["application/vnd.openxmlformats-officedocument.wordprocessingml.document"] = ".docx";
    _types["application/vnd.ms-excel"] = ".xls";
    _types["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx";
    _types["application/vnd.ms-powerpoint"] = ".ppt";
    _types["application/vnd.openxmlformats-officedocument.presentationml.presentation"] = ".pptx";

    // Casos curtos de documentos
    _types["pdf"] = ".pdf";
    _types["plain"] = ".txt";
    _types["csv"] = ".csv";
};

std::string PostHandler::getExtension(const std::string& contentType) {

    if (_types.count(contentType)) {
        return _types[contentType];
    }
    return ".bin";

};

void PostHandler::saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body) {

    std::string extension = getExtension(contentType);

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + extension;
    
    Utils::writeFile(uploadPath, body);
}

std::string PostHandler::getBoundary(const std::string& contentType) {

    size_t pos = contentType.find("boundary=");
    // std::cout << contentType << std::endl;

    if (pos == std::string::npos) {
        std::cout << "Errou" << std::endl;
    }

    pos += 9;
    std::string boundary = contentType.substr(pos);
    boundary =  "--" + boundary;

    return boundary;
}

void PostHandler::parseHeaderLine(const std::string& line, content& result) {
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
    else if (line.find("Content-Type:") != std::string::npos) {
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

void PostHandler::getContents(const std::string& line, content& contents) {

    parseHeaderLine(line, contents);
}


std::string PostHandler::nextLine(const std::string& body, size_t& rangeStart, size_t& endLine) { 

    // rangeStart = endLine + 1;
    endLine = body.find("\n", rangeStart);
    if (endLine == std::string::npos || endLine == body.size()) {
        return "";
    }
    return (body.substr(rangeStart, endLine - rangeStart));
};


void PostHandler::processMuiltPart(const std::string& contentType, const std::string& body, const std::string& uploadPath) {


    std::string boundary = getBoundary(contentType);
    std::string boundaryEnd = boundary + "--";


    content contents;
    size_t rangeStart = 0;
    size_t endLine = 0;
    std::string line;
    while (true) {

        line = nextLine(body, rangeStart, endLine);
        

        if (body.compare(rangeStart, boundary.size() + 2, (boundary + "\r\n")) == 0) {
            rangeStart = endLine + 1;
            line = nextLine(body, rangeStart, endLine);
            getContents(line, contents);
           
            rangeStart = endLine + 1;
            line = nextLine(body, rangeStart, endLine);

            if (line.compare(0, 12, "Content-Type") == 0) {
                getContents(line, contents);
                rangeStart += line.size();
            }


            size_t bpos = body.find(boundary, rangeStart);
            size_t headers_end = body.find("\r\n", rangeStart);

            if (bpos == std::string::npos || headers_end == std::string::npos || headers_end >= bpos)
                continue;
            rangeStart = headers_end + 2;
            size_t partStart = rangeStart;

            if (bpos == std::string::npos)
                continue ;

            std::string partData = body.substr(partStart, bpos - partStart);
            if (partData.size() >= 2 && partData[partData.size()-2] == '\r' && partData[partData.size()-1] == '\n')
                partData.resize(partData.size() - 2);
            else if (partData.size() >= 1 && (partData[partData.size()-1] == '\n' || partData[partData.size()-1] == '\r'))
                partData.resize(partData.size() - 1);
            
            std::string dirUpload = contents.fileName.empty() ? uploadPath + "/" + contents.name + ".txt" : uploadPath + "/" + contents.fileName;
            Utils::writeFile(dirUpload, partData);
            rangeStart = bpos;
        }
        if (body.compare(rangeStart, boundaryEnd.size(), boundaryEnd) == 0) {
            break;
        }
        rangeStart = endLine + 1;
    }

};

void PostHandler::handleRawPost(const Location& location, HttpResponse& response, const std::string& body, const std::string contentType) {
    
    if (_types.count(contentType)) {
        saveFile(contentType, location.getUploadStore(), body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + location.getUploadStore());
    } else {
        response.setStatus(415);
        response.setContentType("text/html");
        response.setBody("415 Unsupported Media Type: The server only supports form uploads.");
    }
};

void PostHandler::handleMultipart(const Location& location, HttpResponse& response, const std::string& body, const std::string contentType) {
    
    try {
        processMuiltPart(contentType, body, location.getUploadStore());
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to ");
    } catch (const std::exception &e) {
        std::cerr << "Error - " << e.what() << std::endl;
        response.setStatus(500);
        response.setContentType("text/html");
        response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
    }
};

void PostHandler::handleFormUrlencoded(const Location& location, HttpResponse& response, const std::string& body) {

    std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".txt";
    if (Utils::writeFile(uploadPath, body)) {
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + uploadPath);
    } else {
        response.setStatus(500);
        response.setContentType("text/html");
        response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
    }
}

HttpResponse PostHandler::process(HttpRequest &request, const Location &location)
{
    HttpResponse response;
    std::string contentType = request.getHeader("Content-Type");
    std::string body = request.getBody();
    response.setHttpVersion(request.getHttpVersion());
    
    initTypes();
    // diferenciar comportamento baseado no Content-Type
    if (contentType == "application/x-www-form-urlencoded") {
        // Processar dados de formulário simples
        handleFormUrlencoded(location, response, body);
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        // Processar upload de arquivo multipart
        // Aqui você precisaria implementar o parsing do corpo multipart
        // Para simplificação, vamos assumir que o arquivo foi salvo com sucesso
        handleMultipart(location, response, body, contentType);
    } else {
        handleRawPost(location, response, body, contentType);
    }
    return response;
}

