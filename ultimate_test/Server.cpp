#include "Server.hpp"

Server::Server(ListeningSocket *ls, std::string const &serverName, std::string const &htmlPath) : _ls(ls), _serverName(serverName), _htmlPath(htmlPath)
{}

Server::~Server()
{}

std::string Server::getServerName() const
{
    return _serverName;
}

Server const *Server::getServer() const
{
    return this;
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
    std::ostringstream contentStream;
	std::string content;
    std::ostringstream headerStream;
    std::ifstream file(_htmlPath.c_str());
    if (!file.is_open()) {
        // Genera una risposta 404 se il file non è trovato con il file default404.html
		file.open("./html/default404.html");
		if (!file.is_open()) {
			throw std::runtime_error("File non trovato e file di default non trovato.");
		}

		contentStream << file.rdbuf();
		file.close();
		content = contentStream.str();

		// Genera l'header HTTP
		headerStream << "HTTP/1.1 404 Not Found\r\n";
		headerStream << "Content-Type: text/html\r\n";
		headerStream << "Content-Length: " << content.size() << "\r\n";
		headerStream << "\r\n";

		response = headerStream.str() + content;
        return response;
    }

    // Leggi il contenuto del file in una stringa
    contentStream << file.rdbuf();
    file.close();
    content = contentStream.str();

    // Genera l'header HTTP
    headerStream << "HTTP/1.1 200 OK\r\n";
    headerStream << "Content-Type: text/html\r\n";
    headerStream << "Content-Length: " << content.size() << "\r\n";
    headerStream << "\r\n";

    // Combina l'header e il contenuto
    response = headerStream.str() + content;

    return response;
}
