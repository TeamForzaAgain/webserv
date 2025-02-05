#include "ServerManager.hpp"

ServerManager::ServerManager() : _activeLs(0)
{}

ServerManager::~ServerManager()
{
    std::cout << "Distruttore ServerManager" << std::endl;
    _pollfds.clear();

    // Distruggere tutti i ClientSocket
    for (std::map<int, ClientSocket *>::iterator it = _clientSockets.begin(); it != _clientSockets.end(); ++it)
    {
        delete it->second;
        it->second = NULL;
    }
    _clientSockets.clear();
    for (size_t i = 0; i < _listeningSockets.size(); i++)
    {
        delete _listeningSockets[i];
    }
    _listeningSockets.clear();
    _servers.clear(); // Distrugge tutti i Server
}

void ServerManager::addPollFd(int fd)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
}

void ServerManager::newServer(int domain, int type, int protocol, int port, u_long interface,
                              ServerConfig const &serverConfig)
{
    // Controlla se esiste già una ListeningSocket per questa porta e indirizzo
    for (size_t i = 0; i < _listeningSockets.size(); i++)
    {
        ListeningSocket *ls = _listeningSockets[i];

        if (ls->getPort() == port && (ls->getInterface() == interface || ls->getInterface() == INADDR_ANY))
        {
            _servers.push_back(Server(ls, serverConfig));
            std::cout << "Nuovo server creato, nome " << serverConfig.serverName << ", IP " << interface << ", porta " << port << "." << std::endl;
            return;
        }
    }

    // Creiamo una nuova ListeningSocket perché non ne esiste una adatta
    _listeningSockets.push_back(new ListeningSocket(domain, type, protocol, port, interface));
    _activeLs++;

    // Creiamo un nuovo server con la nuova ListeningSocket
    _servers.push_back(Server(_listeningSockets.back(), serverConfig));

    // Aggiungiamo la nuova socket al poll
    addPollFd(_listeningSockets.back()->getFd());

    std::cout << "Nuovo server creato, nome " << serverConfig.serverName << ", IP " << interface << ", porta " << port << "." << std::endl;
}

void ServerManager::newClient(int fd, Server const *server)
{
    ClientSocket *newClient = new ClientSocket(fd, server);
    _clientSockets.insert(std::make_pair(fd, newClient));

    std::cout << "Nuovo client connesso: " << fd << std::endl;
}

void ServerManager::registerClient(size_t i)
{
    int newSocket = accept(_pollfds[i].fd, NULL, NULL);
    if (newSocket == -1)
    {
        perror("accept error");
        throw ServerManagerException();
    }

    std::cout << YELLOW << "New connection on socket " << newSocket << " on i: " << i << RESET << std::endl;

    // Trova il Server associato alla ListeningSocket
    Server const *server = NULL;
    for (size_t j = 0; j < _servers.size(); j++)
    {
        if (_servers[j].getListeningSocket()->getFd() == _pollfds[i].fd)
        {
            server = &_servers[j];
            break;
        }
    }

    if (!server)
    {
        std::cerr << "Errore: nessun server trovato per la socket " << _pollfds[i].fd << std::endl;
        close(newSocket);
        return;
    }

    newClient(newSocket, server);
    addPollFd(newSocket);
}

void ServerManager::closeClient(size_t &i)
{
    std::cout << MAGENTA << "Closing socket " << _pollfds[i].fd << RESET << std::endl;
    close(_pollfds[i].fd);
    _clientSockets.erase(_pollfds[i].fd);
    _pollfds.erase(_pollfds.begin() + i);
    i--; // Evita errori di iterazione
}

void ServerManager::readClient(size_t &i)
{
    std::cout << YELLOW << "Reading from socket " << _pollfds[i].fd << RESET << std::endl;

    char tempBuffer[1024];
    int bytesRead = recv(_pollfds[i].fd, tempBuffer, sizeof(tempBuffer) - 1, 0);

    if (bytesRead == 0) // Il client ha chiuso la connessione
    {
        std::cout << MAGENTA << "Client disconnected, closing socket " << _pollfds[i].fd << RESET << std::endl;
        closeClient(i);
        return;
    }
    if (bytesRead == -1)
    {
        perror("recv error");
        return;
    }

    tempBuffer[bytesRead] = '\0';
    _clientSockets[_pollfds[i].fd]->addBuffer(tempBuffer);
    std::cout << BLUE << "Request: " << _clientSockets[_pollfds[i].fd]->getBuffer() << RESET << std::endl;
    int requestStatus = _clientSockets[_pollfds[i].fd]->genResponse(*this);
    if (requestStatus == 1)
        _pollfds[i].events = POLLOUT;
    else
        _pollfds[i].events = POLLIN;

}

void ServerManager::writeClient(size_t &i)
{
    std::string response = _clientSockets[_pollfds[i].fd]->getResponse();
    std::cout << GREEN << "Sending response: " << response << RESET << std::endl;

    int bytesSent = send(_pollfds[i].fd, response.c_str(), response.size(), 0);
    if (bytesSent == -1)
    {
        perror("send error");
        return;
    }

    if (!_clientSockets[_pollfds[i].fd]->getKeepAlive())
    {
        std::cout << MAGENTA << "Client disconnected, closing socket " << _pollfds[i].fd << RESET << std::endl;
        closeClient(i);
        return;
    }
    _pollfds[i].events = POLLIN; // Torna in modalità lettura
}

void ServerManager::run()
{
    while (true)
    {
        std::cout << RED << "Polling..." << RESET << std::endl;
        int activity = poll(_pollfds.data(), _pollfds.size(), -1);

        if (activity == -1)
        {
            perror(strerror(errno));
            throw ServerManagerException();
        }

        std::cout << RED << "Polling done, active sockets: " << activity << RESET << std::endl;
        std::cout << RED << "Active listening sockets: " << _activeLs << RESET << std::endl;

        // **Gestione delle nuove connessioni**
        for (size_t i = 0; i < _activeLs; i++)
        {
            if (_pollfds[i].revents & POLLIN)
                registerClient(i);
        }

        // **Gestione delle socket client**
        for (size_t i = _activeLs; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & (POLLHUP | POLLERR))
                closeClient(i);

            else if (_pollfds[i].revents & POLLIN)
                readClient(i);

            else if (_pollfds[i].revents & POLLOUT)
                writeClient(i);
        }
    }
}

Server const *ServerManager::findServerByHost(const std::string& host, Server const *currentServer)
{
    if (!currentServer)
        return NULL; // Errore: nessun server attuale

    ListeningSocket* listeningSocket = currentServer->getListeningSocket(); // Otteniamo la ListeningSocket attuale

    for (size_t i = 0; i < _servers.size(); i++)
    {
        // Controlliamo se il nome del server corrisponde e se usa la stessa ListeningSocket
        if (_servers[i].getServerName() == host && _servers[i].getListeningSocket() == listeningSocket)
            return &_servers[i];
    }

    return NULL; // Nessun server trovato
}

const char *ServerManager::ServerManagerException::what() const throw()
{
    return "In ServerManagerException";
}