#ifndef INCLUDE_HPP
#define INCLUDE_HPP

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <poll.h>
#include <map>
#include <fstream>
#include <fcntl.h>
#include <csignal>
#include <arpa/inet.h>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>


//ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define ORANGE  "\033[38;5;208m" /* Orange */

#define BUFFERSIZE 4096

#define UPLOAD_DIRECTORY "./uploads/"

struct Methods
{
	bool GET;
	bool POST;
	bool DELETE;
};


// Definizione della struttura Location (location)
struct Location
{
    std::string path;                           // "/images", "/upload", "/" (default)
    std::string root;                           // "/var/www/site1"
    bool autoIndex;                            // true → mostra il contenuto della directory se non c'è un default file
    bool upload;                                // true → se la location è per l'upload di file
    bool cgi;                                   // true → se la location è per l'esecuzione di script CGI
	bool isAlias;                               // true → se la location è un alias
    std::vector<std::string> indexFiles;        // {"index.html", "index.htm", "default.html"}
    Methods allowedMethods;                     // {"GET", "POST", "DELETE"}
	std::map<int, std::string> errorPages;      // {404 -> "custom_404.html"}
};

struct ListenConfig
{
    int port;                                   // 80, 443, etc.
    u_long ip;                                  // INADDR_ANY, INADDR_LOOPBACK, etc.
};

// Definizione della struttura ServerConfig (server {})
struct ServerConfig
{
    std::string hostName;              // Nome del server (es. "example.com")
    int maxBodySize;                        // Dimensione massima del body
    Location defLocation;                   // Location Generale (se nessuna Location Specifica è trovata)
    std::vector<Location> locations;            // Rotte specifiche (location {})
    std::vector<ListenConfig> listens;          // Configurazioni di ascolto};
};

struct HttpRequest
{
    std::string method;                       // "GET", "POST", "DELETE"
    std::string path;                         // "/index.html"
    std::map<std::string, std::string> headers; // "Host", "User-Agent", "Content-Length"
    std::vector<char> body;                         // Contenuto del body (solo per POST/PUT)

    void clear();                               // Resetta la richiesta
};

struct HttpResponse
{
    int statusCode;                          // 200, 404, 500, etc.
    std::string statusMessage;               // "OK", "Not Found", "Internal Server Error"
    std::string contentType;                 // "text/html", "image/jpeg", "application/json"
	std::string body;                         // Contenuto della risposta

    std::string toString() const; // Converte la risposta in stringa HTTP
};

// Definizione della struttura const che associa uno statusCode al suo messaggio`
class HttpStatusCodes
{
public:
    static std::map<int, std::string> createStatusMap()
    {
        std::map<int, std::string> statusMap;
        statusMap[200] = "OK";
        statusMap[201] = "Created";
        statusMap[202] = "Accepted";
        statusMap[204] = "No Content";
        statusMap[301] = "Moved Permanently";
        statusMap[302] = "Found";
        statusMap[303] = "See Other";
        statusMap[304] = "Not Modified";
        statusMap[307] = "Temporary Redirect";
        statusMap[308] = "Permanent Redirect";
        statusMap[400] = "Bad Request";
        statusMap[401] = "Unauthorized";
        statusMap[403] = "Forbidden";
        statusMap[404] = "Not Found";
        statusMap[405] = "Method Not Allowed";
        statusMap[406] = "Not Acceptable";
        statusMap[408] = "Request Timeout";
        statusMap[409] = "Conflict";
        statusMap[411] = "Length Required";
        statusMap[413] = "Payload Too Large";
        statusMap[414] = "URI Too Long";
        statusMap[415] = "Unsupported Media Type";
        statusMap[500] = "Internal Server Error";
        statusMap[501] = "Not Implemented";
        statusMap[502] = "Bad Gateway";
        statusMap[503] = "Service Unavailable";
        statusMap[504] = "Gateway Timeout";
        statusMap[505] = "HTTP Version Not Supported";
        return statusMap;
    }

    static const std::map<int, std::string>& getStatusMap()
    {
        static std::map<int, std::string> statusMap = createStatusMap();
        return statusMap;
    }
};

#endif
