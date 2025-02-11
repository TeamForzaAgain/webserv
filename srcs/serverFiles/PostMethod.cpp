/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostMethod.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fdonati <fdonati@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/10 13:19:43 by tpicchio          #+#    #+#             */
/*   Updated: 2025/02/10 17:01:04 by fdonati          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

HttpResponse Server::genPostResponse(HttpRequest const &request, Location const &location) const
{
	HttpResponse response;

	std::string targetPath = buildFilePath(request, location);
	std::cout << "targetPath: " << targetPath << std::endl;
	
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
