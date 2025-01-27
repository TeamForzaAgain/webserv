#include "Server.hpp"

Server::Server(ListeningSocket *ls, std::string const &serverName, std::string const &htmlPath) : _ls(ls), _serverName(serverName), _htmlPath(htmlPath)
{}

Server::~Server()
{}

std::string Server::getServerName() const
{
    return _serverName;
}

bool Server::operator<(const Server &other) const
{
    // Puoi decidere come ordinare i server. Ecco un esempio basato sul nome del server:
    return _serverName < other._serverName;
}

std::string Server::genResponse(std::string const &request) const
{
    std::string response;

    if (request.find("GET") == std::string::npos) {
        throw std::runtime_error("Solo richieste GET sono supportate.");
    }

    // Leggi il contenuto del file
    std::ifstream file(_htmlPath.c_str());
    if (!file.is_open()) {
        // Genera una risposta 404 se il file non è trovato
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: 45\r\n";
        response += "\r\n";
        response += "<!DOCTYPE html><html><body>Page Not Found</body></html>";
        return response;
    }

    // Leggi il contenuto del file in una stringa
    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    file.close();
    std::string content = contentStream.str();

    // Genera l'header HTTP
    std::ostringstream headerStream;
    headerStream << "HTTP/1.1 200 OK\r\n";
    headerStream << "Content-Type: text/html\r\n";
    headerStream << "Content-Length: " << content.size() << "\r\n";
    headerStream << "\r\n"; // Separatore tra header e corpo

    // Combina l'header e il contenuto
    response = headerStream.str() + content;

    return response;
}
