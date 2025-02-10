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



//ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define ORANGE  "\033[38;5;208m" /* Orange */

#define BUFFERSIZE 524288

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
    std::string path;                    // "/images", "/upload", "/" (default)
    std::string root;           // "/var/www/site1"
    bool dirListing;               // true → mostra il contenuto della directory se non c'è un default f ile
    std::vector<std::string> indexFiles; // {"index.html", "index.htm", "default.html"}
    Methods allowedMethods; // {"GET", "POST", "DELETE"}
	bool isAlias; // true → se la location è un alias
	std::map<int, std::string> errorPages; // {404 -> "custom_404.html"}
};

// Definizione della struttura ServerConfig (server {})
struct ServerConfig
{
    std::string hostName;              // Nome del server (es. "example.com")
    Location defLocation;                   // Location Generale (se nessuna Location Specifica è trovata)
    std::vector<Location> locations;            // Rotte specifiche (location {})
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
    std::string body;                         // Contenuto della risposta

    std::string toString() const; // Converte la risposta in stringa HTTP
};

#endif