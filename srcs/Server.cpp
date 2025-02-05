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

// Rimuove gli slash iniziali e finali da una stringa
// Rimuove lo slash finale se presente
std::string removeTrailingSlash(const std::string& path)
{
    if (!path.empty() && path[path.size() - 1] == '/')
        return path.substr(0, path.size() - 1);
    return path;
}

// Rimuove lo slash iniziale se presente
std::string removeLeadingSlash(const std::string& path)
{
    if (!path.empty() && path[0] == '/')
        return path.substr(1);
    return path;
}


// Unisce due percorsi assicurandosi che abbiano un solo `/`
std::string joinPaths(const std::string& root, const std::string& path)
{
    return removeTrailingSlash(root) + "/" + removeLeadingSlash(path);
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
	std::string targetPath;
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

			if (!it->rootDirectory.empty())
			{
				if (it->alias)
					targetPath = joinPaths(it->rootDirectory, path.substr(it->location.size()));
				else
					targetPath = joinPaths(it->rootDirectory, path);
			}
			else
				targetPath = joinPaths(_defRoute, path);
			break;
		}
	}
	//concatena il path con la root directory
	if (targetPath.empty())
        targetPath = joinPaths(_defRoute, path);
	std::cout <<CYAN<< "Target path: " << targetPath <<RESET<< std::endl;
	return targetPath;
}

std::string Server::genResponse(std::string const &request) const
{
	std::string response;
	std::ostringstream contentStream;
	std::string content;
	std::ostringstream headerStream;
	std::string targetPath = buildFilePath(request);
	std::ifstream file;
	std::string indexPath;

	for (std::vector<std::string>::const_iterator it = _defIndexes.begin(); it != _defIndexes.end(); ++it)
	{
		indexPath = targetPath + *it;
		if (access(indexPath.c_str(), F_OK) != -1)
		{
			file.open(indexPath.c_str());
			break;
		}
	}
	if (!file.is_open())
	{
		indexPath = targetPath + "index.html";
		if (access(indexPath.c_str(), F_OK) != -1)
			file.open(indexPath.c_str());
	}
		
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
