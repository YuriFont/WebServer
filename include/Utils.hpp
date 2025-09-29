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
};

#endif