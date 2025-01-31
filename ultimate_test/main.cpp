#include "ServerManager.hpp"
#include "Include.hpp"

void signalHandler(int signum) {
    std::cout << "\nIntercettato il segnale (" << signum << "). Pulizia e uscita..." << std::endl;
}

ServerConfig fillServer1()
{
	ServerConfig serverConfig;

	serverConfig.serverName = "Server1";

	serverConfig.defaultRoute.location = "";
	serverConfig.defaultRoute.rootDirectory = "./";
	serverConfig.defaultRoute.directoryListing = true;
	serverConfig.defaultRoute.indexes = {};
	serverConfig.defaultRoute.allowedMethods = {};

	Route route;
	route.location = "/test/";
    route.rootDirectory = "";
	route.directoryListing = true;
	route.indexes = {"welcome.html"};
	route.allowedMethods = {"GET"};
	serverConfig.routes.push_back(route);

	return serverConfig;
}

ServerConfig fillServer2()
{
	ServerConfig serverConfig;

	serverConfig.serverName = "Server2";

	serverConfig.defaultRoute.location = "";
	serverConfig.defaultRoute.rootDirectory = "./";
	serverConfig.defaultRoute.directoryListing = true;
	serverConfig.defaultRoute.indexes = {};
	serverConfig.defaultRoute.allowedMethods = {};

	Route route;
	route.location = "/html/";
    route.rootDirectory = "";
	route.directoryListing = true;
	route.indexes = {"index.html"};
	route.allowedMethods = {"GET"};
	serverConfig.routes.push_back(route);

	return serverConfig;
}

int main()
{
	signal(SIGINT, signalHandler);

    ServerManager serverManager;

    try
    {
		ServerConfig serverConfig = fillServer1();
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, serverConfig);
		ServerConfig serverConfig = fillServer2();
		serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_LOOPBACK, serverConfig);
    
        std::cout << "I server sono stati configurati correttamente. Avvio del server manager..." << std::endl;

        serverManager.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Errore durante l'esecuzione del server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
