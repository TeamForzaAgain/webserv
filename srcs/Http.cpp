#include "Include.hpp"

void HttpRequest::clear()
{
    method.clear();
    path.clear();
    headers.clear();
    body.clear();
}

std::string HttpResponse::toString() const
{
	std::ostringstream requestStream;
	
	requestStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	requestStream << "Server: webzerv/TeamForzaAgain\r\n";
	requestStream << "Content-Type: " << contentType << "\r\n";
	requestStream << "Content-Length: " << body.size() << "\r\n";
	requestStream << "\r\n";
	requestStream << body;

	return requestStream.str();
}
