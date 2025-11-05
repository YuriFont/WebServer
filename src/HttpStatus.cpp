#include "../include/HttpStatus.hpp"
#include "../include/Utils.hpp"

HttpStatus::HttpStatus() {
    
    httpStatusMap.insert(std::make_pair(100, "Continue"));
    httpStatusMap.insert(std::make_pair(101, "Switching Protocols"));

    httpStatusMap.insert(std::make_pair(200, "OK"));
    httpStatusMap.insert(std::make_pair(201, "Created"));
    httpStatusMap.insert(std::make_pair(202, "Accepted"));
    httpStatusMap.insert(std::make_pair(203, "Non-Authoritative Information"));
    httpStatusMap.insert(std::make_pair(204, "No Content"));
    httpStatusMap.insert(std::make_pair(205, "Reset Content"));
    httpStatusMap.insert(std::make_pair(206, "Partial Content"));

    httpStatusMap.insert(std::make_pair(300, "Multiple Choices"));
    httpStatusMap.insert(std::make_pair(301, "Moved Permanently"));
    httpStatusMap.insert(std::make_pair(302, "Found"));
    httpStatusMap.insert(std::make_pair(303, "See Other"));
    httpStatusMap.insert(std::make_pair(304, "Not Modified"));
    httpStatusMap.insert(std::make_pair(305, "Use Proxy"));
    httpStatusMap.insert(std::make_pair(307, "Temporary Redirect"));

    httpStatusMap.insert(std::make_pair(400, "Bad Request"));
    httpStatusMap.insert(std::make_pair(401, "Unauthorized"));
    httpStatusMap.insert(std::make_pair(402, "Payment Required"));
    httpStatusMap.insert(std::make_pair(403, "Forbidden"));
    httpStatusMap.insert(std::make_pair(404, "Not Found"));
    httpStatusMap.insert(std::make_pair(405, "Method Not Allowed"));
    httpStatusMap.insert(std::make_pair(406, "Not Acceptable"));
    httpStatusMap.insert(std::make_pair(407, "Proxy Authentication Required"));
    httpStatusMap.insert(std::make_pair(408, "Request Timeout"));
    httpStatusMap.insert(std::make_pair(409, "Conflict"));
    httpStatusMap.insert(std::make_pair(410, "Gone"));
    httpStatusMap.insert(std::make_pair(411, "Length Required"));
    httpStatusMap.insert(std::make_pair(412, "Precondition Failed"));
    httpStatusMap.insert(std::make_pair(413, "Request Entity Too Large"));
    httpStatusMap.insert(std::make_pair(414, "Request-URI Too Long"));
    httpStatusMap.insert(std::make_pair(415, "Unsupported Media Type"));
    httpStatusMap.insert(std::make_pair(416, "Requested Range Not Satisfiable"));
    httpStatusMap.insert(std::make_pair(417, "Expectation Failed"));

    httpStatusMap.insert(std::make_pair(500, "Internal Server Error"));
    httpStatusMap.insert(std::make_pair(501, "Not Implemented"));
    httpStatusMap.insert(std::make_pair(502, "Bad Gateway"));
    httpStatusMap.insert(std::make_pair(503, "Service Unavailable"));
    httpStatusMap.insert(std::make_pair(504, "Gateway Timeout"));
    httpStatusMap.insert(std::make_pair(505, "HTTP Version Not Supported"));
};

HttpStatus::~HttpStatus() {

};

const std::string& HttpStatus::getHttpStatusMensager(const int& status) const {

    static const std::string unknown = "Unknown Status Code";

    if (httpStatusMap.count(status))
        return httpStatusMap.at(status);
    else
        return unknown;
};

int HttpStatus::getCodeStatus() {
    return this->codeStatus;
};

void HttpStatus::setCodeStatus(const int& codeStatus) {
    this->codeStatus = codeStatus;
};

const std::string HttpStatus::makeResponseStatus() {

    return (Utils::toString(this->codeStatus) + " " + this->getHttpStatusMensager(this->codeStatus));
}