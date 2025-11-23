#include "../../include/handlers/PostHandler.hpp"
#include "../../include/utils/Utils.hpp"

HttpResponse PostHandler::process(HttpRequest &request, const Location &location)
{
    std::string contentType = request.getHeader("Content-Type");
    std::string body = request.getBody();
    HttpResponse response;
    

    if (contentType == "image/jpeg") {
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(1) + ".jpg";
        Utils::writeFile(uploadPath, body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + uploadPath);
        return response;
    }

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
        // Tipo de conteúdo não suportado
        response.setStatus(415);
        response.setContentType("text/html");
        response.setBody("415 Unsupported Media Type: The server only supports form uploads.");
    }
    return response;
}

