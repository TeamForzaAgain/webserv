/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteMethod.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/11 12:04:04 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/11 15:39:11 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

HttpResponse Server::genDeleteResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;
	std::string targetPath = buildFilePath(request.path, location);

	if (targetPath[targetPath.size() - 1] == '/')
	{
		return genErrorPage(location, 403, "Forbidden");
	}
	if (targetPath.find("./html/upload/") != 0)
	{
		return genErrorPage(location, 403, "Forbidden");
	}
	if (std::remove(targetPath.c_str()) != 0)
	{
		return genErrorPage(location, 404, "Not Found");
	}
	response.statusCode = 200;
	response.statusMessage = "OK";
	response.contentType = "text/html";
	response.body = "<html><body><h1>File Deleted Successfully</h1></body></html>";
	return response;
}
