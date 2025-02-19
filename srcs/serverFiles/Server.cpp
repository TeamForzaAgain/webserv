/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:23:12 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/19 14:13:25 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(ListeningSocket *ls, ServerConfig const &serverconfig) : _ls(ls), 
_hostName(serverconfig.hostName), _root(serverconfig.defLocation.root), 
_defIndexFiles(serverconfig.defLocation.indexFiles), _defDirListing(serverconfig.defLocation.dirListing),
_errorPages(serverconfig.defLocation.errorPages), _locations(serverconfig.locations), _maxBodySize(serverconfig.maxBodySize)
{}

Location const *Server::getUploadLocation() const
{
	// trovare la location con upload = true se esiste
	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].upload)
			return &_locations[i];
	}
	return NULL;
}

std::string genDefaultErrorPage(HttpResponse &response, int code, const std::string& message)
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
	response.contentType = "text/html";
	return oss.str();
}

HttpResponse Server::genErrorPage(const Location &location, int code, const std::string &message) const
{
	HttpResponse response;
	std::string filePath;
	bool hasCustomPage = false;

	response.statusCode = code;
	response.statusMessage = message;
	response.contentType = "text/html";
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
		response.body = readFileContent(response, filePath);
	if (response.contentType == "Unsupported Media Type")
		response = genErrorPage(location, 415, "Unsupported Media Type");
	if (response.body.empty() || response.contentType != "text/html")
		response.body = genDefaultErrorPage(response, code, message);
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

std::string Server::genResponse(HttpRequest const &request, int statusCode) const
{
	HttpResponse response;
	Location location = findLocation(request);
	if (statusCode == 413)
		return genErrorPage(location, 413, "Payload Too Large").toString();

	if (!isMethodAllowed(location, request.method))
	{
		response = genErrorPage(location, 405, "Method Not Allowed");
		return response.toString();
	}
	if (request.method == "GET")
		response = genGetResponse(request, location);
	if (request.method == "POST")
		response = genPostResponse(request, location);
	else if (request.method == "DELETE")
		response = genDeleteResponse(request, location);

    return response.toString();
}
