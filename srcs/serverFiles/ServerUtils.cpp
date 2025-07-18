#include "Server.hpp"

bool Server::isScript(std::string const &targetPath) const
{
	return ((targetPath.size() >= 3 && targetPath.substr(targetPath.size() - 3) == ".py") ||
			(targetPath.size() >= 3 && targetPath.substr(targetPath.size() - 3) == ".sh"));
}

std::string Server::urlDecodeOnce(const std::string &encoded) const
{
    std::ostringstream decoded;
    
    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            // Prendi i due caratteri dopo il '%'
            char h1 = encoded[i + 1];
            char h2 = encoded[i + 2];
            // Controlla che siano in [0-9A-Fa-f]
            if (isxdigit(h1) && isxdigit(h2)) {
                // Convertili in un carattere
                char buf[3] = { h1, h2, '\0' };
                int hexValue = std::strtol(buf, 0, 16);
                decoded << static_cast<char>(hexValue);
                i += 2; 
                continue;
            }
        }
        // Carattere normale o caso in cui %xx non è valido
        decoded << encoded[i];
    }
    return decoded.str();
}

std::string Server::joinPaths(const std::string &root, const std::string &path) const
{
	std::string cleanRoot = root;
	std::string cleanPath = path;

	if (!cleanRoot.empty() && cleanRoot[cleanRoot.size() - 1] == '/')
		cleanRoot = cleanRoot.substr(0, cleanRoot.size() - 1);

	if (!cleanPath.empty() && cleanPath[0] == '/')
		cleanPath = cleanPath.substr(1);

	return cleanRoot + "/" + cleanPath;
}

std::string Server::findIndexFileContent(const std::string &directory, const std::vector<std::string> &indexFiles) const
{
	HttpResponse response;
	std::string content;
	std::string path;

	for (std::vector<std::string>::const_iterator it = indexFiles.begin(); it != indexFiles.end(); ++it)
	{
		path = joinPaths(directory, *it);
		content = readFileContent(response, path);
		if (response.contentType == "Unsupported Media Type")
			return response.contentType;
		if (!content.empty())
			return content;
	}
	return "";
}

std::string Server::buildFilePath(std::string const &path, Location const &location) const
{
	std::string targetPath;

	if (!location.root.empty())
	{
		if (location.isAlias)
			targetPath = joinPaths(location.root, path.substr(location.path.size()));
		else
			targetPath = joinPaths(location.root, path);
	}
	else
		targetPath = joinPaths(_root, path);
	return targetPath;
}

std::string setContentType(std::string const &extension)
{
	if (extension == "html" || extension == "htm")
		return "text/html";
	else if (extension == "css")
		return "text/css";
	else if (extension == "js")
		return "text/javascript";
	else if (extension == "jpg" || extension == "jpeg")
		return "image/jpeg";
	else if (extension == "png" || extension == "ico")
		return "image/png";
	else if (extension == "gif")
		return "image/gif";
	else if (extension == "mp3")
		return "audio/mpeg";
	else if (extension == "wav")
		return "audio/wav";
	else if (extension == "mp4")
		return "video/mp4";
	else if (extension == "json")
		return "application/json";
	else if (extension == "pdf")
		return "application/pdf";
	else if (extension == "txt")
		return "text/plain";
	return "";
}

std::string Server::readFileContent(HttpResponse &response, std::string const &filePath) const
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";
	std::ostringstream contentStream;
	std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
	response.contentType = setContentType(extension);
	if (response.contentType.empty())
	{
		file.close();
		return "Unsupported Media Type";
	}
	contentStream << file.rdbuf();
	file.close();
	return contentStream.str();
}

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
	defaultLocation.autoIndex = _defAutoIndex;
	defaultLocation.indexFiles = _defIndexFiles;
	defaultLocation.allowedMethods = (Methods){true, false, false};
	defaultLocation.isAlias = false;
	defaultLocation.errorPages = _errorPages;
	return defaultLocation;
}
