#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "WebServer.hpp"

class Location {
    public:
        Location();
        ~Location();

        std::string getPath() const;
        const std::vector<std::string>& getMethods() const;
        bool isMethodAllowed(const std::string& method) const;
        std::string getRoot() const;
        const std::vector<std::string>& getIndex() const;
        bool isAutoindex() const;
        std::string getRedirect() const;
        bool isUploadEnabled() const;
        std::string getUploadStore() const;
        const std::map<std::string, std::string>& getCgi() const;

        void setPath(const std::string &p);
        void setMethods(std::istringstream &iss);
        void setRoot(std::istringstream &iss);
        void setIndex(std::istringstream &iss);
        void setAutoindex(std::istringstream &iss);
        void setRedirect(std::istringstream &iss);
        void setUploadEnabled(bool enable);
        void setUploadStore(std::istringstream &iss);
        void addCgi(std::istringstream &iss);

    private:
        std::string _path;
        std::vector<std::string> _methods;
        std::string _root;
        std::vector<std::string> _index;
        bool _autoindex;
        std::string _redirect;
        int _redirectCode;
        bool _upload_enable;
        std::string _uploadStore;
        std::map<std::string, std::string> _cgi;
};

#endif