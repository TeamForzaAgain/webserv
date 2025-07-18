#include "Server.hpp"

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
	targetPath = urlDecodeOnce(targetPath);
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
