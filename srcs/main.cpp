#include "ServerManager.hpp"
#include "Include.hpp"

void signalHandler(int signum) {
    std::cout <<MAGENTA<< "\nIntercettato il segnale (" << signum << "). Pulizia e uscita..." <<RESET<< std::endl;
	if (signum == SIGCHLD)
	{
		while (waitpid(-1, NULL, WNOHANG) > 0)
			;
	}
}

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
	std::cout << "\tautoindex " << (_defDirListing ? "on" : "off") << ";" << std::endl;
	std::cout << "\tmax_body_size " << _maxBodySize << ";\n" << std::endl;
	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); it++)
		std::cout << "\terror_page " << it->first << " -> " << it->second << ";" << std::endl;
	std::cout << "\n";
	for (size_t i = 0; i < _locations.size(); i++)
	{
		std::cout << "\tlocation " << _locations[i].path << std::endl;
		std::cout << "\t{\n";
		std::cout << "\t\t" << (_locations[i].isAlias ? "alias" : "root") << " " << _locations[i].root << ";" << std::endl;
		std::cout << "\t\tautoindex " << (_locations[i].dirListing ? "on" : "off") << ";" << std::endl;
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

ServerConfig fillServer1()
{
	ServerConfig serverConfig;

	serverConfig.hostName = "Server1";

	serverConfig.defLocation.path = "";
	serverConfig.defLocation.root = "./html/";
	serverConfig.defLocation.dirListing = true;
	serverConfig.maxBodySize = 0;
	serverConfig.defLocation.errorPages[301] = "/errorPages/301.html";
	serverConfig.defLocation.errorPages[403] = "/errorPages/403.html";
	serverConfig.defLocation.errorPages[404] = "/errorPages/404.html";
	serverConfig.defLocation.errorPages[405] = "/errorPages/405.html";
	serverConfig.defLocation.errorPages[415] = "/errorPages/415.html";

	Location location;
	location.path = "/";
	location.root = "./html/";
	location.dirListing = false;
	location.allowedMethods = (Methods){true, true, false};
	location.isAlias = false;
	location.upload = false;
	location.cgi = true;
	serverConfig.locations.push_back(location);

	Location location2;
	location2.path = "/gotta_catch_em_all/";
    location2.root = "./html/";
	location2.dirListing = true;
	location2.upload = false;
	location2.isAlias = false;
	location.cgi = false;
	location2.indexFiles.push_back("index.html");
	location2.allowedMethods = (Methods){true, true, false};
	serverConfig.locations.push_back(location2);

	Location location4;
	location4.path = "/delete/";
    location4.root = "./html/upload/";
	location4.dirListing = true;
	location4.isAlias = true;
	location4.upload = false;
	location4.cgi = false;
	location4.allowedMethods = (Methods){false, false, true};
	serverConfig.locations.push_back(location4);

	Location location5;
	location5.path = "/upload/";
    location5.root = "";
	location5.dirListing = true;
	location5.isAlias = false;
	location5.upload = true;
	location5.cgi = false;
	location5.allowedMethods = (Methods){true, true, true};
	serverConfig.locations.push_back(location5);

	return serverConfig;
}

ServerConfig fillServer2()
{
	ServerConfig serverConfig;

	serverConfig.hostName = "Server2";

	serverConfig.defLocation.path = "";
	serverConfig.defLocation.root = "./html/";
	serverConfig.defLocation.dirListing = true;

	Location location;
	location.path = "/";
	location.root = "./html/";
	location.dirListing = false;
	location.allowedMethods = (Methods){true, false, false};
	location.isAlias = false;
	serverConfig.locations.push_back(location);

	Location location2;
	location2.path = "/server2/";
    location2.root = "/html/";
	location2.dirListing = true;
	location2.indexFiles.push_back("welcome.html");
	location2.allowedMethods = (Methods){true, false, false};
	location2.isAlias = false;
	serverConfig.locations.push_back(location2);
	return serverConfig;
}

int main()
{
	signal(SIGINT, signalHandler);
	signal(SIGCHLD, signalHandler);

    ServerManager serverManager;

    try
    {
		ServerConfig serverConfig = fillServer1();
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, serverConfig);
		ServerConfig serverConfig2 = fillServer2();
		serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_LOOPBACK, serverConfig2);
    
        std::cout << "I server sono stati configurati correttamente. Avvio del server manager..." << std::endl;
		serverManager.printConfig();
        serverManager.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Errore durante l'esecuzione del server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
