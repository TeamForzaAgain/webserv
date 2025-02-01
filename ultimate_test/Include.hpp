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



//ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */

struct Methods
{
	bool GET;
	bool POST;
	bool DELETE;
};

// Definizione della struttura Route (location)
struct Route
{
    std::string location;                    // "/images", "/upload", "/" (default)
    std::string rootDirectory;           // "/var/www/site1"
    bool directoryListing;               // true → mostra il contenuto della directory se non c'è un default f ile
    std::vector<std::string> indexes; // {"index.html", "index.htm", "default.html"}
    Methods allowedMethods; // {"GET", "POST", "DELETE"}
	bool alias; // true → se la location è un alias
};

// Definizione della struttura ServerConfig (server {})
struct ServerConfig
{
    std::string serverName;              // Nome del server (es. "example.com")
    Route defaultRoute;                   // Route Generale (se nessuna Route Specifica è trovata)
    std::vector<Route> routes;            // Rotte specifiche (location {})
    std::map<int, std::string> errorPages; // {404 -> "custom_404.html"}
};

#endif