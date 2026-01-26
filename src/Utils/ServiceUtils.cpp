#include <fstream>
#include <sstream>
#include "ServiceUtils.h"

namespace ServiceUtils
{
    beast::string_view mime_type(beast::string_view path)
    {
        using beast::iequals;
        auto const ext = [&path]
        {
            auto const pos = path.rfind(".");
            if (pos == beast::string_view::npos)
                return beast::string_view{};
            return path.substr(pos);
        }();
        if (iequals(ext, ".htm"))  return "text/html";
        if (iequals(ext, ".html")) return "text/html";
        if (iequals(ext, ".php"))  return "text/html";
        if (iequals(ext, ".css"))  return "text/css";
        if (iequals(ext, ".txt"))  return "text/plain";
        if (iequals(ext, ".js"))   return "application/javascript";
        if (iequals(ext, ".json")) return "application/json";
        if (iequals(ext, ".xml"))  return "application/xml";
        if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
        if (iequals(ext, ".flv"))  return "video/x-flv";
        if (iequals(ext, ".png"))  return "image/png";
        if (iequals(ext, ".jpe"))  return "image/jpeg";
        if (iequals(ext, ".jpeg")) return "image/jpeg";
        if (iequals(ext, ".jpg"))  return "image/jpeg";
        if (iequals(ext, ".gif"))  return "image/gif";
        if (iequals(ext, ".bmp"))  return "image/bmp";
        if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
        if (iequals(ext, ".tiff")) return "image/tiff";
        if (iequals(ext, ".tif"))  return "image/tiff";
        if (iequals(ext, ".svg"))  return "image/svg+xml";
        if (iequals(ext, ".svgz")) return "image/svg+xml";
        return "application/text";
    }

    std::string path_cat(
        beast::string_view base,
        beast::string_view path
    )
    {
        if (base.empty())
            return std::string(path);
        std::string result(base);

#ifdef BOOST_MSVC
        char constexpr pathSeparator = '\\';
        if (result.back() == pathSeparator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
        for (auto& c : result)
            if (c == '/')
                c = pathSeparator;
#else
        char constexpr pathSeparator = '/';
        if (result.back() == pathSeparator)
            result.resize(result.size() - 1);
        result.append(path.data(), path.size());
#endif
        return result;
    }

    bool isValidEmail(const std::string& email)
    {
        const std::regex pattern(
            "(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");

        return std::regex_match(email, pattern);
    }

    bool IsValidPhone(const std::string& phone)
    {
        if (phone.empty() || phone.length() > 20) return false;

        std::string digits;
        for (char c : phone) {
            if (std::isdigit(c)) digits += c;
        }

        if (digits.length() < 10 || digits.length() > 15)
        {
            return false;
        }

        return IsValidInternationalFormat(phone) ||
            IsValidRussianFormat(phone) ||
            IsValidSimpleFormat(phone);
    }

    bool IsValidInternationalFormat(const std::string& phone)
    {
        std::regex pattern(R"(^\+[1-9]\d{0,3}[\s\-]?\(?\d{1,5}\)?[\s\-]?\d{1,15}$)");
        return std::regex_match(phone, pattern);
    }

    bool IsValidRussianFormat(const std::string& phone)
    {
        std::regex pattern(R"(^(\+7|8)?[\s\-]?\(?[489]\d{2}\)?[\s\-]?\d{3}[\s\-]?\d{2}[\s\-]?\d{2}$)");
        return std::regex_match(phone, pattern);
    }

    bool IsValidSimpleFormat(const std::string& phone)
    {
        std::regex pattern(R"(^[\d\s\-\+\(\)]{10,20}$)");
        return std::regex_match(phone, pattern);
    }

    std::string GetConnectionStringFromEnv()
    {
        const char* fullConnectionString = std::getenv("DB_FULL_CONNECTION_STRING");
        if (fullConnectionString)
        {
            return std::string(fullConnectionString);
        }
        return LoadEnvFile();
    }


    std::string LoadEnvFile()
    {
        std::ifstream file(std::filesystem::current_path() / ".." / ".env");
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open .env file");
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("DB_FULL_CONNECTION_STRING=") == 0)
            {
                return line.substr(26);
            }
        }

        throw std::runtime_error("DB_FULL_CONNECTION_STRING not found in .env file");
    }
    
} //ServiceUtils