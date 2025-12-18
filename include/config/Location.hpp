#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "../core/WebServer.hpp"

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
        int getRedirectCode() const;
        bool isUploadEnabled() const;
        bool isCgiEnabled() const;
        std::string getUploadStore() const;
        const std::map<std::string, std::string>& getCgi() const;
        std::string getCgiGlobalExt() const;

        void setPath(const std::string &p);
        void setMethods(std::istringstream &iss);
        void setRoot(std::istringstream &iss);
        void setIndex(std::istringstream &iss);
        void setAutoindex(std::istringstream &iss);
        void setRedirect(std::istringstream &iss);
        void setUploadEnabled(bool enable);
        void setUploadStore(std::istringstream &iss);
        bool hasCgiForExtension(const std::string &ext) const;
        std::string getCgiPathForExtension(const std::string &ext) const;
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
        bool _cgi_enable;
        std::string _uploadStore;
        std::map<std::string, std::string> _cgi;
};

#endif