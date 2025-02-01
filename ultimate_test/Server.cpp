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

std::string Server::buildFilePath(std::string const &request) const
{
	std::string requestLine = request.substr(0, request.find("\r\n"));
	std::string path = requestLine.substr(requestLine.find(" ") + 1, requestLine.rfind(" ") - requestLine.find(" ") - 1);
	std::string method = requestLine.substr(0, requestLine.find(" "));

	std::cout <<CYAN<< "\n\nrequestLine: " << requestLine <<RESET<< std::endl;
	std::cout <<CYAN<< "Method: " << method <<RESET<< std::endl;
	std::cout <<CYAN<< "Path: " << path <<RESET<<"\n"<< std::endl;

	// cerca un match tra le routes location e il path e rispetta c++98
	for (std::vector<Route>::const_iterator it = _routes.begin(); it != _routes.end(); ++it)
	{
		if (path.find(it->location) != std::string::npos)
		{
			std::cout <<CYAN<< "Matched location: " << it->location <<RESET<< std::endl;
			std::cout <<CYAN<< "Root directory: " << it->rootDirectory <<RESET<< std::endl;
			std::cout <<CYAN<< "Directory listing: " << it->directoryListing <<RESET<< std::endl;
			std::cout <<CYAN<< "Indexes: " <<RESET<< std::endl;
			for (std::vector<std::string>::const_iterator it2 = it->indexes.begin(); it2 != it->indexes.end(); ++it2)
				std::cout <<CYAN<< "  " << *it2 <<RESET<< std::endl;
			std::cout <<CYAN<< "Allowed methods: " <<RESET<< std::endl;
			std::cout <<CYAN<< "  GET: " << it->allowedMethods.GET <<RESET<< std::endl;
			std::cout <<CYAN<< "  POST: " << it->allowedMethods.POST <<RESET<< std::endl;
			std::cout <<CYAN<< "  DELETE: " << it->allowedMethods.DELETE <<RESET<< std::endl;
			std::cout <<CYAN<< "Alias: " << it->alias <<RESET<< std::endl;
			break;
		}
	}
	return "./html/index.html";
}

std::string Server::genResponse(std::string const &request) const
{
	std::string response;
	std::ostringstream contentStream;
	std::string content;
	std::ostringstream headerStream;
	std::ifstream file("./html/index.html");

	buildFilePath(request);
	if (request.find("GET") == std::string::npos)
		throw std::runtime_error("Solo richieste GET sono supportate.");
	if (!file.is_open())
        return response404();

	//stampa la prima riga di request

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
