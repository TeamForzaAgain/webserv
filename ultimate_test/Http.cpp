#include "Include.hpp"

void HttpRequest::fromString(const std::string &request)
{
    std::istringstream requestStream(request);
    std::string line;

    // Parsing della prima riga (Metodo, Path, HTTP Version)**
    if (std::getline(requestStream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> method >> path; // Ignoriamo la versione HTTP

        // Convertiamo il metodo in maiuscolo per uniformità
        for (size_t i = 0; i < method.size(); i++)
            method[i] = toupper(method[i]);
    }

    // Parsing degli Header**
    while (std::getline(requestStream, line) && line != "\r")
    {
        size_t separator = line.find(": ");
        if (separator != std::string::npos)
        {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 2);
            headers[key] = value;
        }
    }

    // Parsing del Body (solo per POST/PUT)**
    if (headers.find("Content-Length") != headers.end())
    {
        int contentLength = std::atoi(headers["Content-Length"].c_str());
        if (contentLength > 0)
        {
            char buffer[contentLength + 1];
            requestStream.read(buffer, contentLength);
            buffer[contentLength] = '\0';
            body = std::string(buffer);
        }
    }
}