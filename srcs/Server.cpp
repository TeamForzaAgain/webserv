#include "Server.hpp"
#include "errorResponses.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Server::Server(ListeningSocket *ls, ServerConfig const &serverconfig) : _ls(ls), 
_hostName(serverconfig.hostName), _root(serverconfig.defLocation.root), 
_defIndexFiles(serverconfig.defLocation.indexFiles), _defDirListing(serverconfig.defLocation.dirListing),
_errorPages(serverconfig.defLocation.errorPages), _locations(serverconfig.locations)
{}

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

std::string genDefaultErrorPage(int code, const std::string& message)
{
    std::ostringstream oss;
    oss << "<!DOCTYPE html>\n"
        << "<html lang=\"en\">\n"
        << "<head>\n"
        << "    <title>" << code << " " << message << "</title>\n"
        << "</head>\n"
        << "<body>\n"
        << "    <center>\n"
        << "        <h1>" << code << " " << message << "</h1>\n"
        << "    </center>\n"
        << "    <hr>\n"
        << "    <center>webzerv/TeamForzaAgain</center>\n"
        << "</body>\n"
        << "</html>\n";
    return oss.str();
}

std::string Server::genErrorPage(const Location &location, int code, const std::string &message) const
{
    std::string filePath;
    bool hasCustomPage = false;

    // Verifica se la location ha una pagina di errore personalizzata
    std::map<int, std::string>::const_iterator it = location.errorPages.find(code);
    if (it != location.errorPages.end())
    {
        if (!location.root.empty())
            filePath = joinPaths(location.root, it->second);
        else
            filePath = joinPaths(_root, it->second);
		std::cout <<CYAN<< "Custom error page: " << filePath <<RESET<< std::endl;
        hasCustomPage = true;
    }
    else
    {
        // Verifica se il server ha una pagina di errore personalizzata
        it = _errorPages.find(code);
        if (it != _errorPages.end())
        {
            filePath = joinPaths(_root, it->second);
            hasCustomPage = true;
        }
    }

    if (hasCustomPage)
    {
        std::ifstream file(filePath.c_str());
        if (file.is_open())
        {
            std::ostringstream contentStream;
            contentStream << file.rdbuf();
            file.close();
            return contentStream.str();
        }
        else
        {
            // Se non riesce ad aprire il file personalizzato, usa la pagina di errore predefinita
            return genDefaultErrorPage(code, message);
        }
    }
    else
    {
        // Usa la pagina di errore predefinita
        return genDefaultErrorPage(code, message);
    }
}

// std::string Server::genErrorPage(Location const &location, int code, std::string const &message) const
// {
// 	std::string errorPage;
// 	std::string filePath;
// 	std::map<int, std::string>::const_iterator it = location.errorPages.find(code);

// 	if (it == location.errorPages.end())
// 	{
// 		std::map<int, std::string>::const_iterator it2 = _errorPages.find(code);
// 		if (it2 == _errorPages.end())
// 		{
// 			std::ostringstream oss;
// 			oss << "<!DOCTYPE html>\n"
// 			<< "<html lang=\"en\">\n"
// 			<< "<head>\n"
// 			<< "	<title>" << code << " " << message << "</title>\n"
// 			<< "</head>\n"
// 			<< "<body>\n"
// 			<< "	<center>\n"
// 			<< "		<h1>" << code << " " << message << "</h1>\n"
// 			<< "	</center>\n"
// 			<< "	<hr>\n"
// 			<< "	<center>webzerv/TeamForzaAgain</center>\n"
// 			<< "</body>\n"
// 			<< "</html>\n";
// 			errorPage = oss.str();
// 			return errorPage;
// 		}
// 		else
// 			filePath = joinPaths(_root, it2->second);
// 	}
// 	else if (!location.root.empty())
// 		filePath = joinPaths(location.root, it->second);
// 	else
// 		filePath = joinPaths(_root, it->second);
// 	std::ifstream file(filePath.c_str());
// 	std::ostringstream contentStream;

// 	if (file.is_open())
// 	{
// 		contentStream << file.rdbuf();
// 		errorPage = contentStream.str();
// 		file.close();
// 	}
// 	else
// 	{
// 		std::ostringstream oss;
// 		oss << "<!DOCTYPE html>\n"
// 			<< "<html lang=\"en\">\n"
// 			<< "<head>\n"
// 			<< "	<title>" << code << " " << message << "</title>\n"
// 			<< "</head>\n"
// 			<< "<body>\n"
// 			<< "	<center>\n"
// 			<< "		<h1>" << code << " " << message << "</h1>\n"
// 			<< "	</center>\n"
// 			<< "	<hr>\n"
// 			<< "	<center>webzerv/TeamForzaAgain</center>\n"
// 			<< "</body>\n"
// 			<< "</html>\n";
// 		errorPage = oss.str();
// 	}
// 	return errorPage;
// }

std::string Server::buildFilePath(HttpRequest const &request, Location const &location) const
{
	std::string targetPath;

	if (!location.root.empty())
	{
		if (location.isAlias)
			targetPath = joinPaths(location.root, request.path.substr(location.path.size()));
		else
			targetPath = joinPaths(location.root, request.path);
	}
	else
		targetPath = joinPaths(_root, request.path);
	return targetPath;
}

std::string Server::genDirListing(std::string const &path, Location const &location) const
{
	std::string dirListing;
	std::ostringstream oss;
	DIR *dir;
	struct dirent *ent;
	struct stat st;
	std::string filePath;

	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		return genErrorPage(location, 403, "Forbidden");
	}
	oss << "<!DOCTYPE html>"
		<< "<html lang=\"en\">"
		<< "<head>"
		<< "<title>Index of " << path << "</title>"
		<< "</head>"
		<< "<body>"
		<< "<center>"
		<< "<h1>Index of " << path << "</h1>"
		<< "<hr>"
		<< "<table>";
	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") == 0)
			continue;
		filePath = joinPaths(path, ent->d_name);
		if (stat(filePath.c_str(), &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
				oss << "<tr><td><a href=\"" << ent->d_name << "/\">" << ent->d_name << "/</a></td></tr>";
			else
				oss << "<tr><td><a href=\"" << ent->d_name << "\">" << ent->d_name << "</a></td></tr>";
		}
	}
	oss << "</table>"
		<< "<hr>"
		<< "<center>webzerv/TeamForzaAgain</center>"
		<< "</body>"
		<< "</html>";
	dirListing = oss.str();
	closedir(dir);
	return dirListing;
}

bool findIndexFile(const std::string &directory, const std::vector<std::string> &indexFiles, std::string &indexContent)
{
    for (std::vector<std::string>::const_iterator it = indexFiles.begin(); it != indexFiles.end(); ++it)
    {
        std::string indexPath = joinPaths(directory, *it);
        std::ifstream file(indexPath.c_str());
        if (file.is_open())
        {
            std::ostringstream contentStream;
            contentStream << file.rdbuf();
            indexContent = contentStream.str();
            file.close();
            return true;
        }
    }
    return false;
}



HttpResponse Server::genGetResponse(HttpRequest const &request) const
{
	HttpResponse response;
	std::ifstream file;
	std::ostringstream contentStream;

	response.location = findLocation(request);
	if (response.location.allowedMethods.GET == false)
	{
		response.body = genErrorPage(response.location, 403, "Forbidden");
		response.statusCode = 403;
		response.statusMessage = "Forbidden";
		return response;
	}

	std::string targetPath = buildFilePath(request, response.location);

	std::cout <<CYAN<< "Matched location: " << response.location.path <<RESET<< std::endl;
	std::cout <<CYAN<< "Root directory: " << response.location.root <<RESET<< std::endl;
	std::cout <<CYAN<< "Directory listing: " << response.location.dirListing <<RESET<< std::endl;
	std::cout <<CYAN<< "Target path: " << targetPath <<RESET<< std::endl;
	std::cout <<CYAN<< "Indexes: " <<RESET<< std::endl;
	for (std::vector<std::string>::const_iterator it = response.location.indexFiles.begin(); it != response.location.indexFiles.end(); ++it)
		std::cout <<CYAN<< "  " << *it <<RESET<< std::endl;
	std::cout <<CYAN<< "Allowed methods: " <<RESET<< std::endl;
	std::cout <<CYAN<< "  GET: " << response.location.allowedMethods.GET <<RESET<< std::endl;
	std::cout <<CYAN<< "  POST: " << response.location.allowedMethods.POST <<RESET<< std::endl;
	std::cout <<CYAN<< "  DELETE: " << response.location.allowedMethods.DELETE <<RESET<< std::endl;
	std::cout <<CYAN<< "Alias: " << response.location.isAlias <<RESET<< std::endl;
	if (targetPath[targetPath.size() - 1] == '/')
	{
		bool foundIndex = false;
		std::vector<std::string>::const_iterator it;
		std::string indexPath;

		for (it = response.location.indexFiles.begin(); it != response.location.indexFiles.end(); ++it)
		{
			indexPath = joinPaths(targetPath, *it);
			file.open(indexPath.c_str());
			if (file.is_open())
			{
				foundIndex = true;
				break;
			}
		}
		if (!foundIndex)
		{
			for (it = _defIndexFiles.begin(); it != _defIndexFiles.end(); ++it)
			{
				indexPath = joinPaths(targetPath, *it);
				file.open(indexPath.c_str());
				if (file.is_open())
				{
					foundIndex = true;
					break;
				}
			}
			if (!foundIndex)
			{
				indexPath = joinPaths(targetPath, "index.html");
				file.open(indexPath.c_str());
				if (file.is_open())
					foundIndex = true;
			}
		}
		if (foundIndex)
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
			contentStream << file.rdbuf();
			response.body = contentStream.str();
			file.close();
		}
		else if (response.location.dirListing)
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
			response.body = genDirListing(targetPath, response.location);
		}
		else
		{
			response.body = genErrorPage(response.location, 403, "Forbidden");
			response.statusCode = 403;
			response.statusMessage = "Forbidden";
		}
	}
	else
	{
		struct stat st;
		if (stat(targetPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		{
			response.body = genErrorPage(response.location, 301, "Moved Permanently");
			response.statusCode = 301;
			response.statusMessage = "Moved Permanently";
			return response;
		}

		file.open(targetPath.c_str());
		if (!file.is_open())
		{
			response.body = genErrorPage(response.location, 404, "Not Found");
			response.statusCode = 404;
			response.statusMessage = "Not Found";
		}
		else
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
			contentStream << file.rdbuf();
			response.body = contentStream.str();
			file.close();
		}
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
