#include "ServerManager.hpp"
#include <iostream>
#include <cstdlib>
#include <string>
#include <arpa/inet.h>

int main()
{
    // Ottieni il nome dell'utente corrente
    const char* user = std::getenv("USER");
    if (!user) {
        std::cerr << "Errore: impossibile determinare l'utente corrente." << std::endl;
        return 1;
    }

    // Determina il percorso base in base all'utente
    std::string basePath;
    if (std::string(user) == "tpicchio") {
        basePath = "/nfs/homes/tpicchio/Desktop/Corso_42/Progetti/webserv/ultimate_test/html/";
    } else if (std::string(user) == "fdonati") {
        basePath = "/nfs/homes/fdonati/cazzi/webserver/webserv/ultimate_test/html/";
    } else {
        std::cerr << "Errore: utente non riconosciuto." << std::endl;
        return 1;
    }

    ServerManager serverManager;

    try
    {
        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY, "Server8080", basePath + "welcome.html");

        serverManager.newServer(AF_INET, SOCK_STREAM, 0, 9090, INADDR_ANY, "Server9090", basePath + "info.html");

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
