#include "Include.hpp"
#include "ServerManager.hpp"

void Server::printServer() const
{
	std::cout << "server" << std::endl;
	std::cout << "{\n";
	std::cout << "\tserver_name " << _hostName << ";" << std::endl;
	std::cout << "\tlisten " << _ls->getInterface() << ":" << _ls->getPort() << ";" << std::endl;
	std::cout << "\troot " << _root << ";" << std::endl;
	std::cout << "\tindex ";
	for (size_t i = 0; i < _defIndexFiles.size(); i++)
		std::cout << _defIndexFiles[i] << " ";
	std::cout << ";" << std::endl;
	std::cout << "\tautoindex " << (_defAutoIndex ? "on" : "off") << ";" << std::endl;
	std::cout << "\tmax_body_size " << _maxBodySize << ";\n" << std::endl;
	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); it++)
		std::cout << "\terror_page " << it->first << " -> " << it->second << ";" << std::endl;
	std::cout << "\n";
	for (size_t i = 0; i < _locations.size(); i++)
	{
		std::cout << "\tlocation " << _locations[i].path << std::endl;
		std::cout << "\t{\n";
		std::cout << "\t\t" << (_locations[i].isAlias ? "alias" : "root") << " " << _locations[i].root << ";" << std::endl;
		std::cout << "\t\tautoindex " << (_locations[i].autoIndex ? "on" : "off") << ";" << std::endl;
		std::cout << "\t\tupload " << (_locations[i].upload ? "on" : "off") << ";" << std::endl;
		std::cout << "\t\tcgi " << (_locations[i].cgi ? "on" : "off") << ";" << std::endl;
		std::cout << "\t\tallowed_methods " << (_locations[i].allowedMethods.GET ? "GET " : "") << (_locations[i].allowedMethods.POST ? "POST " : "") << (_locations[i].allowedMethods.DELETE ? "DELETE " : "") << ";" << std::endl;
		std::cout << "\t\tindex ";
		for (size_t j = 0; j < _locations[i].indexFiles.size(); j++)
			std::cout << _locations[i].indexFiles[j] << " ";
		std::cout << ";" << std::endl;
		for (std::map<int, std::string>::const_iterator it = _locations[i].errorPages.begin(); it != _locations[i].errorPages.end(); it++)
			std::cout << "\t\terror_page "<< it->first << " -> " << it->second << ";" << std::endl;
		std::cout << "\n\t}\n\n";
	}
	std::cout << "}\n\n";
}

void ServerManager::printConfig() const
{
	std::cout << YELLOW << "\nConfiguration File:\n" << CYAN << std::endl;
	for (size_t i = 0; i < _servers.size(); i++)
		_servers[i].printServer();
	std::cout << RESET<< std::endl;
}
