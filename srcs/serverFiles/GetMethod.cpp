/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethod.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:05:01 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/11 15:38:02 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
		return genErrorPage(location, 404, "Not Found");
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
	response.contentType = "text/html";
	response.body = oss.str();
	closedir(dir);
	return response;
}

HttpResponse Server::genGetResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;
	std::ifstream file;
	std::ostringstream contentStream;

	std::string targetPath = buildFilePath(request.path, location);
	if (targetPath[targetPath.size() - 1] == '/')
	{
		response.body = findIndexFileContent(targetPath, location.indexFiles);
		if (response.body.empty())
			response.body = findIndexFileContent(targetPath, _defIndexFiles);
		if (response.body.empty())
			response.body = readFileContent(response, joinPaths(targetPath, "index.html"));

		if (response.body == "Unsupported Media Type")
			return genErrorPage(location, 415, "Unsupported Media Type");

		if (!response.body.empty())
		{
			response.statusCode = 200;
			response.statusMessage = "OK";
		}
		else if (location.dirListing)
		{
			response = genDirListing(targetPath, location);
		}
		else if (opendir(targetPath.c_str()) == NULL)
		{
			response = genErrorPage(location, 404, "Not Found");
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
		response.body = readFileContent(response, targetPath);
		if (response.body == "Unsupported Media Type")
			return genErrorPage(location, 415, "Unsupported Media Type");
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
