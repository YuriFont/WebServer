#include "../../include/handlers/CgiHandler.hpp"


CgiHandler::CgiHandler(const ServerConfig& config, const HttpRequest& request, const Location& location, int client_fd): _config(config), _request(request), _location(location), _response(NULL), _isFinish(false) {
    this->client_fd = client_fd;
};

CgiHandler::CgiHandler(const CgiHandler& other): _config(other._config), _request(other._request), _location(other._location), _response(NULL), _isFinish(other._isFinish) {

    if (other._response != NULL) {
        this->_response = new HttpResponse(*other._response);
    }
};


CgiHandler::~CgiHandler() {

    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
};

IMethodHandler* CgiHandler::clone() const {
    return new CgiHandler(*this);
};

bool CgiHandler::isCgi() const {
     return true; 
};

void CgiHandler::handleData(const std::string& chunk) {

    _body.append(chunk);
    if ((int)_body.size() == _request.getContentLength()) {
        std::cout << "body size: " << _body.size() << " Request length: " << _request.getContentLength() << std::endl;
        _isFinish = true;
    }
};

bool CgiHandler::isFinished() {
    return (_isFinish);
};

std::string CgiHandler::getExtensionCgi() {
    
    std::string rawPath = _request.getPath();
    // Remover query string
    size_t q = rawPath.find('?');
    std::string path = (q != std::string::npos) ? rawPath.substr(0, q) : rawPath;
    size_t dot = path.find_last_of('.');
    std::string extension = path.substr(dot);

    return extension;
}

HttpResponse& CgiHandler::buildCgiResponse() {
    HttpResponse *resp = new HttpResponse();

    resp->setBody("<h1>deu certo</h1>");
    resp->setStatus(200);
    return *resp;
};


HttpResponse& CgiHandler::getResponse() {

    if (_response != NULL)
        return *_response;
    HttpResponse& response = buildCgiResponse();
    return response;
};

std::vector<std::string> CgiHandler::buildCgiEnv(const HttpRequest &request, const Location &location, const std::string &scriptPath){
    (void) location;
    std::vector<std::string> env;
    env.push_back("REQUEST_METHOD=" + request.getMethod());
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    std::cout << "query:" << request.getQueryString() << std::endl;
    env.push_back("QUERY_STRING=" + request.getQueryString());
    env.push_back("CONTENT_LENGTH=" + Utils::toString(_body.size()));
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

HttpResponse& CgiHandler::responseHTTP(const std::string &output)
{
    size_t headerEnd = output.find("\r\n\r\n");
    size_t sepLen = 4;

    if (headerEnd == std::string::npos) {
        headerEnd = output.find("\n\n");
        sepLen = 2;
    }
    _response = new HttpResponse();

    if (headerEnd == std::string::npos) {
        _response->setStatus(500);
        _response->setContentType("text/html");
        _response->setBody("<h1>CGI Internal Error</h1>");
        return *_response;
    }

    std::string headers = output.substr(0, headerEnd);
    std::string body    = output.substr(headerEnd + sepLen);

    int status = 200;

    // --- Parse Status ---
    size_t posStatus = headers.find("Status:");
    if (posStatus != std::string::npos) {
        size_t start = posStatus + 7;
        size_t end   = headers.find("\n", start);
        std::string statusLine = Utils::trim(headers.substr(start, end - start));
        int code = std::atoi(statusLine.c_str());
        if (code >= 100 && code <= 599)
            status = code;
    }

    // --- Parse Content-Type ---
    std::string contentType = "text/html";
    size_t posCT = headers.find("Content-Type:");
    if (posCT != std::string::npos) {
        size_t start = posCT + 13;
        size_t end = headers.find("\n", start);
        contentType = Utils::trim(headers.substr(start, end - start));
    }

    // --- Parse Location ---
    size_t posLoc = headers.find("Location:");
    bool hasLocation = false;
    if (posLoc != std::string::npos) {
        size_t start = posLoc + 9;
        size_t end = headers.find("\r\n", start);
        std::string loc = Utils::trim(headers.substr(start, end - start));
        _response->setHeader("Location", loc);
        hasLocation = true;
    }

    _response->setStatus(status);
    _response->setContentType(contentType);

    // --- SPECIAL RULE FOR REDIRECTS ---
    if (status >= 300 && status < 400 && hasLocation) {
        _response->setBody("");               // body EMPTY
        _response->setHeader("Content-Length", "0");
        _response->setConnectionClose(true);
        return *_response;
    }

    // Normal CGI _response
    _response->setBody(body);
    _response->setConnectionClose(true);
    return *_response;
}

CgiProcess* CgiHandler::startCgi(){

    std::string path = Utils::buildPathRequisition(_location.getPath(), _location.getRoot(), _request.getPath());
    std::string extension = getExtensionCgi();
    CgiProcess::CgiStatus status = CgiProcess::CGI_OK;

    if (!Utils::validTraversalPath(path, _location.getRoot())) {
        status = CgiProcess::CGI_FORBIDDEN;
    } else {

        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            if (errno == EACCES)
                status = CgiProcess::CGI_FORBIDDEN;
            else if (errno == ENOENT)
                status = CgiProcess::CGI_NOT_FOUND;
            else
                status = CgiProcess::CGI_INTERNAL_ERROR;
        } else if (!S_ISREG(info.st_mode)) {
            status = CgiProcess::CGI_FORBIDDEN;
        }
        else if (access(path.c_str(), X_OK) != 0)
                status = CgiProcess::CGI_FORBIDDEN;
    }


    // Se aconteceu algum erro
    if (status != CgiProcess::CGI_OK) {
        CgiProcess* cgi = new CgiProcess();
        cgi->client_fd = client_fd;
        cgi->input = _body;   // body JÁ LIDO
        cgi->stdin_closed = true;
        cgi->stdout_closed = true;
        cgi->status = status;
        return cgi;
    }

    
    std::string interpreter;
    if (_config.hasGlobalCGI && _config.hasExtGlobalCgi(extension))
        interpreter = _config.extAndPath.find(extension)->second;
    else
        interpreter = _location.getCgiPathForExtension(extension);

    int inPipe[2]; //servidor -> CGI (STDIN do CGI)
    int outPipe[2]; //CGI -> servidor (STDOUT do CGI)

    
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1)
        throw std::runtime_error("Error creating pipe");
    fcntl(inPipe[1], F_SETFL, O_NONBLOCK);
    fcntl(outPipe[0], F_SETFL, O_NONBLOCK);
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error("Error forking process");
    if(pid == 0) //processo filho - executa o CGI
        spawnCgiChild(_request, _location, path, interpreter, inPipe, outPipe);

    //Processo pai: lê a resposta
    close(inPipe[0]);
    close(outPipe[1]);

    CgiProcess* cgi = new CgiProcess();
    cgi->stdin_fd = inPipe[1];
    cgi->stdout_fd = outPipe[0];
    cgi->pid = pid;
    cgi->client_fd = client_fd;
    cgi->input = _body;   // body JÁ LIDO
    cgi->stdin_closed = false;

    return cgi;

}
