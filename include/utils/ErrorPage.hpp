#ifndef ERROR_PAGE_HPP
#define ERROR_PAGE_HPP

#include <string>
#include "../utils/Utils.hpp"

class ErrorPage {
    public:
        static std::string build(int code);
};

#endif