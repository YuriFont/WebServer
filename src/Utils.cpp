#include "../include/Utils.hpp"

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

std::string Utils::getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".ico") != std::string::npos) return "image/vnd.microsoft.icon";
    if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return "image/jpeg";
    return "text/html"; // default
}
