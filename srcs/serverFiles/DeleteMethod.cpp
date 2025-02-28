#include "Server.hpp"

// Decodifica UNA volta le sequenze %xx in caratteri (ad es. %20 -> ' ').
std::string urlDecodeOnce(const std::string &encoded) {
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

HttpResponse Server::genDeleteResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;
	std::string targetPath = buildFilePath(request.path, location);

	if (targetPath[targetPath.size() - 1] == '/')
	{
		if (location.path == getUploadLocation()->path)
		{
			response = genAutoIndex(targetPath, location);
			return response;
		}
		response = genErrorPage(location, 403);
		return response;
	}
	if (location.path != getUploadLocation()->path)
	{
		response = genErrorPage(location, 403);
		return response;
	}
	// sostituisci i %20 con spazi
	targetPath = urlDecodeOnce(targetPath);
	std::cout << YELLOW << "Deleting file: " << targetPath << RESET << std::endl;
	if (std::remove(targetPath.c_str()) != 0)
	{
		return genErrorPage(location, 404);
	}
	response.statusCode = 200;
	response.statusMessage = "OK";
	response.contentType = "text/html";
	response.body = "<html><body><h1>File Deleted Successfully</h1></body></html>";
	return response;
}
