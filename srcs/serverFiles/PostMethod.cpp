/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostMethod.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:19:43 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/12 13:07:50 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

HttpResponse Server::genPostResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;

	std::string targetPath = buildFilePath(request.path, location);
	
	//controlla se la location e' quella di upload usando getUploadLocation()
	Location const *uploadLocation = getUploadLocation();
	if (uploadLocation && uploadLocation->path == location.path)
	{
		std::string filename;
		std::map<std::string, std::string>::const_iterator it = request.headers.find("filename");
		if (it != request.headers.end()) 
    		filename = it->second;
		else
			filename = "uploaded_file.bin";
		
		//controlla se il file e' stato auploadatp correttamente
		struct stat buffer;
		if (stat(targetPath.c_str(), &buffer) == 0)
		{
			//il file e' stato caricato correttamente
			response.statusCode = 201;
			response.statusMessage = "Created";
			response.contentType = "text/html";
			response.body = "<html><body><h1>File Uploaded Successfully</h1></body></html>";
		}
		else
		{
			//il file non e' stato caricato correttamente
			response.statusCode = 500;
			response.statusMessage = "Internal Server Error";
			response.contentType = "text/html";
			response.body = "<html><body><h1>Internal Server Error</h1></body></html>";
		}
	}
	else
	{
		//la location non e' quella di upload
		response = genErrorPage(location, 403, "Forbidden");
	}
	
	return response;
		
	
}
