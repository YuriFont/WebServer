#include "../../include/handlers/CgiHandler.hpp"

std::vector<std::string> CgiHandler::buildCgiEnv(const HttpRequest &request, const Location &location, const std::string &scriptPath){
    (void) location;
    std::vector<std::string> env;
    env.push_back("REQUEST_METHOD=" + request.getMethod());
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    std::cout << "query:" << request.getQueryString() << std::endl;
    env.push_back("QUERY_STRING=" + request.getQueryString());
    env.push_back("CONTENT_LENGTH=" + Utils::toString(request.getBody().size()));
    env.push_back("CONTENT_TYPE=" + request.getHeader("Content-Type"));
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.getHttpVersion());
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    env.push_back("SERVER_NAME=localhost");
    env.push_back("SERVER_PORT=8080");
    env.push_back("PATH_INFO=" + request.getPath());
    env.push_back("REDIRECT_STATUS=200"); //Tem que puxar o retorno do processo aqui
    return env;
}

void CgiHandler::spawnCgiChild(const HttpRequest &request, const Location &location,
                               const std::string &path, const std::string &interpreter,
                               int inPipe[2], int outPipe[2]){
    dup2(inPipe[0], STDIN_FILENO);
    dup2(outPipe[1], STDOUT_FILENO);

    close(inPipe[1]);
    close(outPipe[0]);

    //Monta o ambiente CGI
    std::vector<std::string> env = buildCgiEnv(request, location, path);
    char **envp = Utils::vecToCharArray(env); 

    //Argumentos
    char *argv[] = {
        const_cast<char *>(interpreter.c_str()),
        const_cast<char *>(path.c_str()),
        NULL};

    execve(interpreter.c_str(), argv, envp);

    // Se chegou aqui, execve falhou
    std::cerr << "CGI execve failed: " << strerror(errno) << std::endl;
    Utils::freeCharArray(envp);
    exit(EXIT_FAILURE);
}

std::string CgiHandler::readCgiOutput(int outPipe[2], pid_t pid)
{
    close(outPipe[1]);
    std::string output;
    char buffer[4096];
    ssize_t bytes;

    while ((bytes = read(outPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytes);

    close(outPipe[0]);
    waitpid(pid, NULL, 0);

    return output;
}

HttpResponse CgiHandler::responseHTTP(const std::string &output, HttpResponse &response)
{
    size_t headerEnd = output.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        headerEnd = output.find("\n\n"); //Sempre é \r\n\r\n

    //Cabeçalho invalido
    if (headerEnd == std::string::npos)
    {
        response.setStatus(500);
        response.setContentType("text/html");
        response.setBody("<h1>CGI Internal Error</h1>");
        return response;
    }

    std::string headers = output.substr(0, headerEnd);
    std::string body = output.substr(headerEnd + 4);

    // Procura por Content-Type
    std::string contentType = "text/html";
    size_t pos = headers.find("Content-Type:");
    if (pos != std::string::npos)
    {
        size_t start = pos + 13;
        size_t end = headers.find("\r\n", start);
        contentType = Utils::trim(headers.substr(start, end - start));
    }

    response.setStatus(200);
    response.setContentType(contentType);
    response.setBody(body);
    response.setConnectionClose(true);
    return response;
}

HttpResponse CgiHandler::process(const HttpRequest &request, const Location &location, std::string extension){
    HttpResponse response;

    std::string path = Utils::buildPathRequisition(location.getPath(), location.getRoot(), request.getPath());
    std::string interpreter = location.getCgiPathForExtension(extension);

    int inPipe[2]; //servidor -> CGI (STDIN do CGI)
    int outPipe[2]; //CGI -> servidor (STDOUT do CGI)
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
        throw std::runtime_error("Error creating pipe");
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("Error forking process");
    if(pid == 0) //processo filho - executa o CGI
        spawnCgiChild(request, location, path, interpreter, inPipe, outPipe);

    //Processo pai: lê a resposta
    close(inPipe[0]);
    close(outPipe[1]);

    //Envia o corpo do POST (se houver)
    if (request.getMethod() == "POST" && !request.getBody().empty())
        write(inPipe[1], request.getBody().c_str(), request.getBody().size());
    close(inPipe[1]);

    //Lê saída do CGI
    std::string output = readCgiOutput(outPipe, pid);

    //Monta a resposta HTTP
    response = responseHTTP(output, response);
    return response;
}