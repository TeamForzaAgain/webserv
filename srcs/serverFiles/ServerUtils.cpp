/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:14:01 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/11 15:17:22 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::string Server::joinPaths(const std::string& root, const std::string& path) const
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
	std::string content;
	std::string path;

	for (std::vector<std::string>::const_iterator it = indexFiles.begin(); it != indexFiles.end(); ++it)
	{
		path = joinPaths(directory, *it);
		content = readFileContent(path);
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

std::string Server::readFileContent(const std::string &filePath) const
{
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return "";
	std::ostringstream contentStream;
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
	defaultLocation.dirListing = _defDirListing;
	defaultLocation.indexFiles = _defIndexFiles;
	defaultLocation.allowedMethods = (Methods){true, false, false};
	defaultLocation.isAlias = false;
	defaultLocation.errorPages = _errorPages;
	return defaultLocation;
}
