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
