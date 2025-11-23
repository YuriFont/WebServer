#include "../../include/utils/Utils.hpp"

Utils::Utils() {}

Utils::~Utils() {}

std::string Utils::trim(const std::string &s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    size_t last  = s.find_last_not_of(" \t\n\r");

    if (first == std::string::npos || last == std::string::npos)
        return "";

    return s.substr(first, last - first + 1);
}

std::string Utils::toString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

size_t Utils::toSizeT(const std::string &s) {
    return static_cast<size_t>(strtoul(s.c_str(), NULL, 10));
}

int Utils::countWords(std::istringstream &iss) {
    std::istringstream issTemp(iss.str());
    std::string word;
    int count = 0;

    while (issTemp >> word)
        count++;

    return count;
}

bool Utils::isValidUrl(const std::string &url)
{
    if (url.find("http://") == 0) {
        if (url.size() <= 7)
            return false;
    }
    else if (url.find("https://") == 0) {
        if (url.size() <= 8)
            return false;
    }
    else
        return false;

    std::string rest = url.substr(url.find("://") + 3);
    if (rest.find('.') == std::string::npos)
        return false;

    if (rest[rest.size()-1] == '.')
        return false;

    return true;
}

std::string Utils::buildPathRequisition(const std::string& locationPath, const std::string& rootPath, const std::string& requestPath) {

    std::string path = requestPath;
    std::string result = rootPath;
    path.replace(0, locationPath.size(), "");
    if (!result.empty() && result[result.size() - 1] != '/')
        result += '/';
    if (!path.empty() && path[0] == '/')
        path = path.substr(1);
    result += path;

    return result;
}

std::string Utils::getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".ico") != std::string::npos) return "image/vnd.microsoft.icon";
    if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return "image/jpeg";
    return "text/html"; // default
}

// Read a file from disk and return its contents as a string
// Ler o arquivo do disco e retornar o conteúdo em uma string
bool Utils::readFile(const std::string &path, std::string &out)
{
    int fd;
    char buffer[1024];
    ssize_t bytesRead;

    fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
        return false;
    out.clear();
    while((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
        out.append(buffer, bytesRead);
    close(fd);
    return true;
}

// Generate a basic HTML directory listing (autoindex)
// Gerar um HTML básico com a lista de diretórios (autoindex)
std::string Utils::generateAutoindex(const std::string &dirPath, const std::string &urlPath)
{
    std::string html = "<html><head><title>Index of " + urlPath + "</title></head><body>";
    html += "<h1>Index of " + urlPath + "</h1><ul>";

    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
    {
        html += "<li>Permission denied</li></ul></body></html>";
        return html;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == ".")
            continue;

        // ignora o pai ("..") mas pode incluir se quiser
        // ignora o pai ("..")
        if (name == "..")
            continue;

        html += "<li><a href=\"" + urlPath;
        if (urlPath.size() == 0 || urlPath[urlPath.size() - 1] != '/')
            html += "/";
        html += name + "\">" + name + "</a></li>";
    }

    closedir(dir);
    html += "</ul></body></html>";
    return html;
}

bool Utils::writeFile(const std::string &path, const std::string &data)
{
    std::ofstream outFile(path.c_str(), std::ios::binary);
    if (!outFile.is_open())
        return false;

    outFile.write(data.c_str(), data.size());
    return outFile.good();
}
char **Utils::vecToCharArray(const std::vector<std::string> &vec)
{
    char **arr = new char *[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); i++)
        arr[i] = strdup(vec[i].c_str());
    arr[vec.size()] = NULL;
    return arr;
}
