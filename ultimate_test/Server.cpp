#include "Server.hpp"
#include "errorResponses.hpp"

Server::Server(ListeningSocket *ls, ServerConfig const &serverconfig) : _ls(ls)
{
	_serverName = serverconfig.serverName;
	_defRoute = serverconfig.defaultRoute.rootDirectory;
	_defIndexes = serverconfig.defaultRoute.indexes;
	_defDirListing = serverconfig.defaultRoute.directoryListing;
	_routes = serverconfig.routes;
}

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

ListeningSocket *Server::getListeningSocket() const
{
    return _ls;
}

bool Server::operator<(const Server &other) const
{
    return _serverName < other._serverName;
}

// std::string Server::buildFilePath(std::string const &request) const
// {
	
// 	return "";
// }

std::string Server::genResponse(std::string const &request) const
{
	std::string response;
	std::ostringstream contentStream;
	std::string content;
	std::ostringstream headerStream;
	std::ifstream file("./html/index.html");

	if (request.find("GET") == std::string::npos)
		throw std::runtime_error("Solo richieste GET sono supportate.");
	if (!file.is_open())
        return response404();

    contentStream << file.rdbuf();
    file.close();
    content = contentStream.str();
    headerStream << "HTTP/1.1 200 OK\r\n";
    headerStream << "Content-Type: text/html\r\n";
    headerStream << "Content-Length: " << content.size() << "\r\n";
    headerStream << "\r\n";
    response = headerStream.str() + content;
    return response;
}
