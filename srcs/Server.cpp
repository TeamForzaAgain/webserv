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

// Unisce due percorsi assicurandosi che abbiano un solo `/`
std::string joinPaths(const std::string& root, const std::string& path)
{
	std::string cleanRoot = root;
	std::string cleanPath = path;

	if (!cleanRoot.empty() && cleanRoot[cleanRoot.size() - 1] == '/')
		cleanRoot = cleanRoot.substr(0, cleanRoot.size() - 1);

	if (!cleanPath.empty() && cleanPath[0] == '/')
		cleanPath = cleanPath.substr(1);

	return cleanRoot + "/" + cleanPath;
}

std::string readFileContent(const std::string &filePath)
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";
	std::ostringstream contentStream;
	contentStream << file.rdbuf();
	file.close();
	return contentStream.str();
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

HttpResponse Server::genErrorPage(const Location &location, int code, const std::string &message) const
{
	HttpResponse response;
	std::string filePath;
	bool hasCustomPage = false;

	response.statusCode = code;
	response.statusMessage = message;
	// Verifica se la location ha una pagina di errore personalizzata
	std::map<int, std::string>::const_iterator it = location.errorPages.find(code);
	if (it != location.errorPages.end())
	{
		if (!location.root.empty())
			filePath = joinPaths(location.root, it->second);
		else
			filePath = joinPaths(_root, it->second);
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
		response.body = readFileContent(filePath);
	if (response.body.empty())
		response.body = genDefaultErrorPage(code, message);
	return response;
}

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

HttpResponse Server::genDirListing(std::string const &path, Location const &location) const
{
	HttpResponse response;
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
	response.statusCode = 200;
	response.statusMessage = "OK";
	response.body = oss.str();
	closedir(dir);
	return response;
}

std::string findIndexFileContent(const std::string &directory, const std::vector<std::string> &indexFiles)
{
	std::vector<std::string>::const_iterator it;
	std::string content;
	std::string path;

	for (it = indexFiles.begin(); it != indexFiles.end(); ++it) {
		path = joinPaths(directory, *it);
		content = readFileContent(path);
		if (!content.empty())
			return content;
	}
	return "";
}

HttpResponse Server::genGetResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;
	std::ifstream file;
	std::ostringstream contentStream;

	std::string targetPath = buildFilePath(request, location);
	if (targetPath[targetPath.size() - 1] == '/')
	{
		response.body = findIndexFileContent(targetPath, location.indexFiles);
		if (response.body.empty())
			response.body = findIndexFileContent(targetPath, _defIndexFiles);
		if (response.body.empty())
			response.body = readFileContent(joinPaths(targetPath, "index.html"));

		if (!response.body.empty())
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
		}
		else if (location.dirListing)
		{
			response = genDirListing(targetPath, location);
		}
		else
		{
			response = genErrorPage(location, 403, "Forbidden");
		}
	}
	else
	{
		struct stat st;
		if (stat(targetPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
		{
			return genErrorPage(location, 301, "Moved Permanently");
		}
		response.body = readFileContent(targetPath);
		if (response.body.empty())
		{
			response = genErrorPage(location, 404, "Not Found");
		}
		else
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
		}
	}
	return response;
}

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

HttpResponse Server::genPostResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;

	std::string targetPath = buildFilePath(request, location);
	
	// Controlla che il path sia una directory
	struct stat st;
	/* if (stat(targetPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
	{

		response.body = genErrorPage(response.location, 301, "Moved Permanently");
		response.statusCode = 301;
		response.statusMessage = "Moved Permanently";
		return response;
	} */
	// Controlla se il Content-Type è multipart/form-data
	if (request.headers.find("Content-Type") != request.headers.end() &&
		request.headers.find("Content-Type")->second.find("multipart/form-data") != std::string::npos)
	{
		if (request.headers.find("filename") != request.headers.end())
		{
			std::string filename = request.headers.find("filename")->second;
			std::string sourcePath = "./uploads/" + filename;

			// Controlla che il file esista
			if (stat(sourcePath.c_str(), &st) != 0)
			{
				return genErrorPage(location, 404, "File Not Found");
			}

			// Creazione del processo figlio
			pid_t pid = fork();
			if (pid == -1)
			{
				return genErrorPage(location, 500, "Internal Server Error");
			}
			else if (pid == 0) // Processo figlio
			{
				// Esegui mv per spostare il file
				char *args[] = {(char *)"/bin/mv", (char *)sourcePath.c_str(), (char *)targetPath.c_str(), NULL};
				execve("/bin/mv", args, NULL);
				_exit(1); // Se execve fallisce
			}
			else // Processo padre
			{
				int status;
				waitpid(pid, &status, 0);
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
				{
					response.body = "File uploaded successfully.";
					response.statusCode = 200;
					response.statusMessage = "OK";
				}
				else
				{
					response = genErrorPage(location, 500, "Internal Server Error");
				}
			}
		}
	}

	return response;
}

bool isMethodAllowed(const Location &location, const std::string &method)
{
	if (method == "GET")
		return location.allowedMethods.GET;
	if (method == "POST")
		return location.allowedMethods.POST;
	if (method == "DELETE")
		return location.allowedMethods.DELETE;
	return false;
}


std::string Server::genResponse(HttpRequest const &request) const
{
	HttpResponse response;
	Location location = findLocation(request);

	if (!isMethodAllowed(location, request.method))
	{
		response = genErrorPage(location, 405, "Method Not Allowed");
		return response.toString();
	}
	if (request.method == "GET")
		response = genGetResponse(request, location);
	if (request.method == "POST")
		response = genPostResponse(request, location);
	// else if (request.method == "DELETE")
	// 	response = genDeleteResponse(request);

    return response.toString();
}
