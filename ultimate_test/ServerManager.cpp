#include "ServerManager.hpp"

ServerManager::ServerManager() : _activeLs(0)
{}

ServerManager::~ServerManager()
{
	//pulire la memoria
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
                                const std::string& serverName = "default", std::string const &htmlPath = "/html/welcome.html")
{
	for (std::map<Server, ListeningSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        ListeningSocket *ls = it->second;

        // Caso 1: La socket ha lo stesso indirizzo e porta o esiste già una socket che ascolta su INADDR_ANY
        if (ls->getPort() == port && (ls->getInterface() == interface || ls->getInterface() == INADDR_ANY))
        {
            _servers.insert(std::make_pair(Server(ls, serverName, htmlPath), ls));
            std::cout << "Nuovo server creato, nome "<< serverName << ", IP " << interface << ", porta " << port << "." << std::endl;
            return;
        }
    }

    // Se non esiste una socket compatibile, creane una nuova
    ListeningSocket *newLs = new ListeningSocket(domain, type, protocol, port, interface);
    _activeLs++;
    _servers.insert(std::make_pair(Server(newLs, serverName, htmlPath), newLs));

    // Aggiungi la nuova ListeningSocket al poll
    addPollFd(newLs->getFd());

std::cout << "Nuovo server creato, nome "<< serverName << ", IP " << interface << ", porta " << port << "." << std::endl;}

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
    for (std::map<Server, ListeningSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        if (it->second->getFd() == _pollfds[i].fd)
        {
            server = &(it->first);
            break;
        }
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

    // Se la richiesta è completa, genera la risposta
    if (_clientSockets[_pollfds[i].fd]->parseEndMessage())
    {
        std::cout << BLUE << "Request: " << _clientSockets[_pollfds[i].fd]->getBuffer() << RESET << std::endl;
        _clientSockets[_pollfds[i].fd]->genResponse(*this);
        _pollfds[i].events = POLLOUT;
    }
    else
    {
        std::cout << YELLOW << "Not ended Buffer: " << _clientSockets[_pollfds[i].fd]->getBuffer() << RESET << std::endl;
    }
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

Server* ServerManager::findServerByHost(const std::string& host, Server const *currentServer)
{
    if (!currentServer)
        return NULL; // **Errore: non abbiamo un server di riferimento!**

    ListeningSocket* listeningSocket = _servers[*currentServer]; // Otteniamo la ListeningSocket attuale

    for (std::map<Server, ListeningSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        // **Trova un server con lo stesso nome e la stessa ListeningSocket**
        if (it->first.getServerName() == host && it->second == listeningSocket)
        {
            return const_cast<Server*>(&(it->first)); // **Trovato il server giusto**
        }
    }

    return NULL; // **Nessun server trovato**
}

const char *ServerManager::ServerManagerException::what() const throw()
{
    return "In ServerManagerException";
}