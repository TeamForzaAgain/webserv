// main.cpp
#include "ServerManager.hpp"
#include <iostream>

int main()
{
    // Crea un'istanza di ServerManager
    ServerManager serverManager;

    try
    {
        // Configura il primo server su porta 8080
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, "Server8080", "");

        // Configura il secondo server su porta 9090
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_ANY, "Server9090", "/html/info.html");

        std::cout << "I server sono stati configurati correttamente. Avvio del server manager..." << std::endl;

        // Esegui il ciclo principale del server manager
        serverManager.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Errore durante l'esecuzione del server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
