#include "ServerManager.hpp"
#include "Include.hpp"

void signalHandler(int signum) {
    std::cout << "\nIntercettato il segnale (" << signum << "). Pulizia e uscita..." << std::endl;
}

ServerConfig fillServer1()
{
	ServerConfig serverConfig;

	serverConfig.hostName = "Server1";

	serverConfig.defLocation.path = "";
	serverConfig.defLocation.root = "./html/";
	serverConfig.defLocation.dirListing = true;
	serverConfig.defLocation.errorPages[301] = "/errorPages/301.html";
	serverConfig.defLocation.errorPages[403] = "/errorPages/403.html";
	serverConfig.defLocation.errorPages[404] = "/errorPages/404.html";
	serverConfig.defLocation.errorPages[405] = "/errorPages/405.html";
	serverConfig.defLocation.errorPages[415] = "/errorPages/415.html";

	Location location;
	location.path = "/server1/";
    location.root = "";
	location.dirListing = true;
	location.upload = false;
	location.isAlias = false;
	location.indexFiles.push_back("info.html");
	location.allowedMethods = (Methods){true, false, false};
	location.isAlias = false;
	serverConfig.locations.push_back(location);

	Location location2;
	location2.path = "/server1/";
    location2.root = "";
	location2.dirListing = true;
	location2.upload = false;
	location2.isAlias = false;
	location2.indexFiles.push_back("welcome.html");
	location2.allowedMethods = (Methods){true, false, false};
	serverConfig.locations.push_back(location2);

	Location location3;
	location3.path = "/favicon.ico";
	location3.root = "./html/";
	location3.dirListing = false;
	location3.upload = false;
	location3.indexFiles.push_back("favicon.ico");
	location3.allowedMethods = (Methods){true, false, false};
	serverConfig.locations.push_back(location3);

	Location location4;
	location4.path = "/upload/";
	location4.root = "./html/";
	location4.dirListing = true;
	location4.upload = true;
	location4.allowedMethods = (Methods){true, true, true};
	location4.isAlias = false;
	serverConfig.locations.push_back(location4);

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

    ServerManager serverManager;

    try
    {
		ServerConfig serverConfig = fillServer1();
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, serverConfig);
		ServerConfig serverConfig2 = fillServer2();
		serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_LOOPBACK, serverConfig2);
    
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
