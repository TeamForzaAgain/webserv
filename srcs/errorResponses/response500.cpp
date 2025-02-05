/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response500.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tpicchio <tpicchio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 11:36:58 by tpicchio          #+#    #+#             */
/*   Updated: 2025/01/29 11:41:01 by tpicchio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "errorResponses.hpp"

// This function is used to generate a simple 500 response when even the file default500.html has a problem.
std::string basicResponse500()
{
	std::ostringstream contentStream;
	std::string content;

	contentStream << "<!DOCTYPE html>"
	<< "<html lang=\"it\">"
	<< "<head>"
	<< "<meta charset=\"UTF-8\">"
	<< "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	<< "<title>500 Internal Server Error</title>"
	<< "<style>"
	<< "body {"
	<< "  display: flex;"
	<< "  justify-content: center;"
	<< "  align-items: center;"
	<< "  height: 20vh;"
	<< "  margin: 0;"
	<< "  background-color: #f8f9fa;"
	<< "  color: #333;"
	<< "  font-family: Arial, sans-serif;"
	<< "}"
	<< "h1 {"
	<< "  font-size: 3em;"
	<< "  margin-bottom: 0.5em;"
	<< "}"
	<< "p {"
	<< "  font-size: 1.5em;"
	<< "}"
	<< "</style>"
	<< "</head>"
	<< "<body>"
	<< "<div>"
	<< "<h1>500 Internal Server Error</h1>"
	<< "<p>Qualcosa è andato storto sul server. Riprova più tardi.</p>"
	<< "</div>"
	<< "</body>"
	<< "</html>";
	content = contentStream.str();
	return content;
}

std::string response500()
{
	std::ostringstream contentStream;
	std::string response;
	std::ostringstream headerStream;
	std::ifstream file;


	file.open("./html/default500.html");
	if (!file.is_open())
		response = basicResponse500();
	else
	{
		contentStream << file.rdbuf();
		file.close();
		response = contentStream.str();
	}

	headerStream << "HTTP/1.1 500 Internal Server Error\r\n";
    headerStream << "Content-Type: text/html\r\n";
    headerStream << "Content-Length: " << response.size() << "\r\n";
    headerStream << "\r\n";
	response = headerStream.str() + response;
	return response;
}
