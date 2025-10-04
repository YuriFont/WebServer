#ifndef UTILS_HPP
# define UTILS_HPP

#include "WebServer.hpp"

class Utils {
    private:
        Utils();
        ~Utils();

    public:
        static std::string trim(const std::string &s);
        static std::string toString(int n);
        static size_t toSizeT(const std::string &s);
        static int countWords(std::istringstream &iss);
        static bool isValidUrl(const std::string &url);
};

#endif