#include "Include.hpp"

void HttpRequest::clear()
{
    method.clear();
    path.clear();
    headers.clear();
	cookies.clear();
    body.clear();
}

void HttpRequest::parseCookies()
{
	cookies.clear();
	std::map<std::string, std::string>::iterator it = headers.find("Cookie");
	if (it != headers.end())
	{
		std::string cookieHeader = it->second;
		std::stringstream ss(cookieHeader);
		std::string cookie;
		while (std::getline(ss, cookie, ';'))
		{
			size_t eqPos = cookie.find('=');
			if (eqPos != std::string::npos)
			{
				std::string key = cookie.substr(0, eqPos);
				std::string value = cookie.substr(eqPos + 1);
				key.erase(0, key.find_first_not_of(' '));
				key.erase(key.find_last_not_of(' ') + 1);
				value.erase(0, value.find_first_not_of(' '));
				value.erase(value.find_last_not_of(' ') + 1);
				cookies[key] = value;
			}
		}
	}
}

void HttpResponse::setCookie(const std::string &name, const std::string &value, const std::string &attributes)
{
	std::cout << "🔹 Setting Set-Cookie header: " << name << "=" << value << "; " << attributes << std::endl;
	cookies[name] = value + (attributes.empty() ? "" : "; " + attributes);
}

std::string HttpResponse::toString() const
{
	std::ostringstream requestStream;
	
	requestStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	requestStream << "Server: webzerv/TeamForzaAgain\r\n";
	requestStream << "Content-Type: " << contentType << "\r\n";
	requestStream << "Content-Length: " << body.size() << "\r\n";
	if (!location.empty())
		requestStream << "Location: " << location << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); ++it)
    {
        std::cout << "🔹 Sending Set-Cookie: " << it->first << "=" << it->second << std::endl;
        requestStream << "Set-Cookie: " << it->first << "=" << it->second << "\r\n";
    }

	requestStream << "\r\n";
	requestStream << body;

	return requestStream.str();
}
