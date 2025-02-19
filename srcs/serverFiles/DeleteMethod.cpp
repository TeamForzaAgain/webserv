/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteMethod.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tpicchio <tpicchio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 12:04:04 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/19 15:09:20 by tpicchio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

HttpResponse Server::genDeleteResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;
	std::string targetPath = buildFilePath(request.path, location);

	if (targetPath[targetPath.size() - 1] == '/')
	{
		if (location.path == getUploadLocation()->path)
		{
			response = genDirListing(targetPath, location);
			std::cout << CYAN << "Body: " << response.body << RESET << std::endl;
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
