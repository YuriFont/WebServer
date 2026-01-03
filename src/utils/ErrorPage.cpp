#include "../../include/utils/ErrorPage.hpp"
#include <sstream>

static const std::string bodyTemplate =
"<!DOCTYPE html>"
    "<html>"
    "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>{{CODE}} {{TITLE}}</title>"
        "<style>"
            "body{font-family:Arial,sans-serif;background:#f5f7fa;margin:0;padding:0;}"
            ".box{max-width:600px;margin:120px auto;padding:30px;background:#fff;"
            "border-radius:8px;box-shadow:0 4px 12px rgba(0,0,0,.12);text-align:center;}"
            ".code{font-size:48px;font-weight:bold;color:#3498db;}"
        "</style>"
    "</head>"
        "<body>"
            "<div class=\"box\">"
            "<div class=\"code\">{{ICON}} {{CODE}}</div>"
            "<h1>{{TITLE}}</h1>"
            "<p>{{MESSAGE}}</p>"
        "</div>"
    "</body>"
"</html>";

static void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

static std::string getTitle(int code)
{
    if (code == 400) 
        return "Bad Request";
    if (code == 403) 
        return "Forbidden";
    if (code == 404) 
        return "Not Found";
    if (code == 405) 
        return "Method Not Allowed";
    if (code == 413) 
        return "Payload Too Large";
    if (code == 504)
        return "Gateway Timeout";

    return "Internal Server Error";
}

static std::string getMessage(int code)
{
    if (code == 400) 
        return "The server could not understand the request.";
    if (code == 403) 
        return "You do not have permission to access this resource.";
    if (code == 404) 
        return "The requested resource could not be found.";
    if (code == 405) 
        return "The HTTP method is not allowed for this resource.";
    if (code == 413) 
        return "The request body exceeds the allowed size.";
    if (code == 504)
        return "The request timed out. The server took too long to respond.";

    return "An unexpected error occurred on the server.";
}

static std::string getIcon(int code)
{
    if (code == 404) 
        return "🔍";
    if (code == 403) 
        return "🔒";
    if (code == 413) 
        return "📦";
    if (code >= 500) 
        return "❌";
    return "⚠️";
}


std::string ErrorPage::build(int code) {
    
    std::string html = bodyTemplate;

    replaceAll(html, "{{CODE}}", Utils::toString(code));
    replaceAll(html, "{{TITLE}}", getTitle(code));
    replaceAll(html, "{{MESSAGE}}", getMessage(code));
    replaceAll(html, "{{ICON}}", getIcon(code));

    return html;
};