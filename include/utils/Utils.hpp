#ifndef UTILS_HPP
# define UTILS_HPP

#include "../core/WebServer.hpp"

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
        static std::string getContentType(const std::string& path);
        static std::string buildPathRequisition(const std::string& locationPath, const std::string& rootPath, const std::string& requestPath);
        static bool readFile(const std::string &path, std::string &out);
        static std::string generateAutoindex(const std::string &dirPath, const std::string &urlPath);
        static bool writeFile(const std::string &path, const std::string &data);
        static void freeCharArray(char **envp);
        static char **vecToCharArray(const std::vector<std::string> &vec);
        static std::string extractValue(const std::string& line, const std::string& key);
        static std::string normalizePath(const std::string &path);
        static bool validTraversalPath(const std::string &rawPath, const std::string &rootPath);
        static std::string getExtension(const std::string &path);
};

#endif