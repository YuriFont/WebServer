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

HttpResponse PostHandler::process(HttpRequest &request, const Location &location)
{
    std::string contentType = request.getHeader("Content-Type");
    std::string body = request.getBody();
    HttpResponse response;
    response.setHttpVersion(request.getHttpVersion());


    // std::cout << request.getBuffer() << std::endl;
    
    initTypes();

    // diferenciar comportamento baseado no Content-Type
    if (contentType == "application/x-www-form-urlencoded") {
        // Processar dados de formulário simples
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".txt";
        std::cout << "Body (" << body.size() << " bytes): [" << body << "]" << std::endl;
        if (Utils::writeFile(uploadPath, body)) {
            std::cout << request.getPath() << std::endl;
            response.setStatus(201);
            response.setContentType("text/html");
            response.setBody("File uploaded successfully to " + uploadPath);
        } else {
            response.setStatus(500);
            response.setContentType("text/html");
            response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
        }
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        // Processar upload de arquivo multipart
        // Aqui você precisaria implementar o parsing do corpo multipart
        // Para simplificação, vamos assumir que o arquivo foi salvo com sucesso
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".bin";
        if (Utils::writeFile(uploadPath, body)) {
            response.setStatus(201);
            response.setContentType("text/html");
            response.setBody("File uploaded successfully to " + uploadPath);
        } else {
            response.setStatus(500);
            response.setContentType("text/html");
            response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
        }
    } else {
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
    }
    return response;
}

