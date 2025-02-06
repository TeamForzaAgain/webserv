#include "Server.hpp"
#include "errorResponses.hpp"

Server::Server(ListeningSocket *ls, ServerConfig const &serverconfig) : _ls(ls), 
_hostName(serverconfig.hostName), _root(serverconfig.defLocation.root), 
_defIndexFiles(serverconfig.defLocation.indexFiles), _defDirListing(serverconfig.defLocation.dirListing),
_errorPages(serverconfig.defLocation.errorPages), _locations(serverconfig.locations)
{
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

/*buildFilePath
if (!it->root.empty())
			{
				if (it->isAlias)
					targetPath = joinPaths(it->root, request.path.substr(it->path.size()));
				else
					targetPath = joinPaths(it->root, request.path);
			}
			else
				targetPath = joinPaths(_root, request.path);

//concatena il path con la root directory
	if (targetPath.empty())
        targetPath = joinPaths(_defRoute, path);
	std::cout <<CYAN<< "Target path: " << targetPath <<RESET<< std::endl;
*/

 
Location Server::findLocation(HttpRequest const &request) const
{
	const Location* bestMatch = NULL;
	size_t bestMatchLength = 0;

	std::cout <<CYAN<< "Method: " << request.method <<RESET<< std::endl;
	std::cout <<CYAN<< "Path: " << request.path <<RESET<<"\n"<< std::endl;
	for (std::vector<Location>::const_iterator it = _locations.begin(); it != _locations.end(); ++it)
	{
		if (request.path.rfind(it->path, 0) == 0) // Verifica se path inizia con it->path
		{
			size_t currentMatchLength = it->path.length();
			if (currentMatchLength > bestMatchLength) // Trova il match più lungo
			{
				bestMatch = &(*it);
				bestMatchLength = currentMatchLength;
			}
		}
	}

	if (bestMatch)
		return *bestMatch;

	Location defaultLocation;
	defaultLocation.path = "";
	defaultLocation.root = _root;
	defaultLocation.dirListing = _defDirListing;
	defaultLocation.indexFiles = _defIndexFiles;
	defaultLocation.allowedMethods = (Methods){true, false, false};
	defaultLocation.isAlias = false;
	defaultLocation.errorPages = _errorPages;
	return defaultLocation;
}

HttpResponse Server::genGetResponse(HttpRequest const &request) const
{
	HttpResponse response;
	response.location = findLocation(request);

	std::cout <<CYAN<< "Matched location: " << response.location.path <<RESET<< std::endl;
	std::cout <<CYAN<< "Root directory: " << response.location.root <<RESET<< std::endl;
	std::cout <<CYAN<< "Directory listing: " << response.location.dirListing <<RESET<< std::endl;
	std::cout <<CYAN<< "Indexes: " <<RESET<< std::endl;
	for (std::vector<std::string>::const_iterator it = response.location.indexFiles.begin(); it != response.location.indexFiles.end(); ++it)
		std::cout <<CYAN<< "  " << *it <<RESET<< std::endl;
	std::cout <<CYAN<< "Allowed methods: " <<RESET<< std::endl;
	std::cout <<CYAN<< "  GET: " << response.location.allowedMethods.GET <<RESET<< std::endl;
	std::cout <<CYAN<< "  POST: " << response.location.allowedMethods.POST <<RESET<< std::endl;
	std::cout <<CYAN<< "  DELETE: " << response.location.allowedMethods.DELETE <<RESET<< std::endl;
	std::cout <<CYAN<< "Alias: " << response.location.isAlias <<RESET<< std::endl;
	// std::string filePath = buildFilePath(request);
	std::ifstream file("html/test/info.html");
	std::ostringstream contentStream;

	if (!file.is_open())
	{
		response.statusCode = 404;
		response.statusMessage = "Not Found";
		file.close();
		file.open("html/default404.html");
		contentStream << file.rdbuf();
		response.body = contentStream.str();
	}
	else
	{
		response.statusCode = 200;
		response.statusMessage = "OK";
		contentStream << file.rdbuf();
		response.body = contentStream.str();
		file.close();
	}
	return response;
}

std::string Server::genResponse(HttpRequest const &request) const
{
	HttpResponse response;

	if (request.method == "GET")
		response = genGetResponse(request);
	// else if (request.method == "POST")
	// 	response = genPostResponse(request);
	// else if (request.method == "DELETE")
	// 	response = genDeleteResponse(request);

    return response.toString();
}
