#include "ServerManager.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>

void signalHandler(int signum) {
    std::cout << "\nIntercettato il segnale (" << signum << "). Pulizia e uscita..." << std::endl;
}

int main()
{
	signal(SIGINT, signalHandler);

    ServerManager serverManager;

    try
    {
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, "Server8080", "./html/welcome.html");
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_LOOPBACK, "Server8080_2", "./html/info.html");
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_LOOPBACK, "Server8080_3", "./html/info.html");
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_LOOPBACK, "Server8080_4", "./html/info.html");
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_LOOPBACK, "Server8080_5", "./html/info.html");
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_LOOPBACK, "Server8080_6", "./html/info.html");

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
