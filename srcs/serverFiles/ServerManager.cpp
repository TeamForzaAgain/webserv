#include "ServerManager.hpp"

extern volatile sig_atomic_t g_signal_status;

ServerManager::ServerManager() : _activeLs(0)
{}

ServerManager::~ServerManager()
{
    _pollfds.clear();

    for (size_t i = 0; i < _pollfds.size(); i++)
    {
        delete _clientSockets[_pollfds[i].fd];
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
            std::cout << "[ServerManager] Nuovo server creato, nome " << serverConfig.hostName << ", IP " << inet_ntoa(*(struct in_addr*)&interface) << ", porta " << port << "." << std::endl;
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

    std::cout << "[ServerManager] Nuovo server creato, nome " << serverConfig.hostName << ", IP " << inet_ntoa(*(struct in_addr*)&interface) << ", porta " << port << "." << std::endl;
}

void ServerManager::newClient(int fd, Server *server)
{
    ClientSocket *newClient = new ClientSocket(fd, server);
    _clientSockets.insert(std::make_pair(fd, newClient));

}

void ServerManager::registerClient(size_t i)
{
    int newSocket = accept(_pollfds[i].fd, NULL, NULL);
    if (newSocket == -1)
    {
        perror("accept error");
        throw ServerManagerException();
    }

	std::cout << YELLOW << "[ServerManager] Nuova connessione sulla socket " << newSocket << RESET << std::endl;

    // Trova il Server associato alla ListeningSocket
    Server *server = NULL;
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
        std::cerr << RED << "[ServerManager] Errore: nessun server trovato per la socket " << _pollfds[i].fd << std::endl;
        close(newSocket);
        return;
    }

    newClient(newSocket, server);
    addPollFd(newSocket);
}

void ServerManager::closeClient(size_t &i)
{
	std::cout << YELLOW << "[ServerManager] Chiusura della socket " << _pollfds[i].fd << RESET << std::endl;
    close(_pollfds[i].fd);
    delete _clientSockets[_pollfds[i].fd];
    _clientSockets[_pollfds[i].fd] = NULL;
    _clientSockets.erase(_pollfds[i].fd);
    _pollfds.erase(_pollfds.begin() + i);
    i--;
}

void ServerManager::readClient(size_t &i)
{
	std::cout << YELLOW << "[ServerManager] Lettura dalla socket " << _pollfds[i].fd << RESET << std::endl;
    char tempBuffer[BUFFERSIZE];
    int bytesRead = recv(_pollfds[i].fd, tempBuffer, BUFFERSIZE - 1, 0);

    if (bytesRead == 0) // Il client ha chiuso la connessione
    {
        closeClient(i);
        return;
    }
    if (bytesRead == -1)
    {
        perror("recv error");
        closeClient(i);
        return;
    }

    tempBuffer[bytesRead] = '\0';
    _clientSockets[_pollfds[i].fd]->addBuffer(tempBuffer, bytesRead);
    _clientSockets[_pollfds[i].fd]->genResponse(*this);
    if (_clientSockets[_pollfds[i].fd]->getStatus() > 0)
        _pollfds[i].events = POLLOUT;
    else
        _pollfds[i].events = POLLIN;
    
    _clientSockets[_pollfds[i].fd]->setLastActivity();
}

void ServerManager::writeClient(size_t &i)
{
    std::string response = _clientSockets[_pollfds[i].fd]->getResponse();
	std::cout << YELLOW << "[ServerManager] Invio risposta al client: " << _pollfds[i].fd << RESET << std::endl;

    int bytesSent = send(_pollfds[i].fd, response.c_str(), response.size(), 0);
    if (bytesSent == -1)
    {
        perror("send error");
        closeClient(i);
        return;
    }
    if (bytesSent == 0)
    {
        // Significa che la connessione è chiusa o non più utilizzabile.
        closeClient(i);
        return;
    }

    if (!_clientSockets[_pollfds[i].fd]->getKeepAlive())
    {
        std::cout << YELLOW << "[ServerManager] Client disconnesso, " << RESET;
        closeClient(i);
        return;
    }
    _pollfds[i].events = POLLIN; // Torna in modalità lettura
}

void ServerManager::run()
{
    while (true)
    {
        std::cout << GREEN << "[ServerManager] Polling..." << RESET << std::endl;
        poll(_pollfds.data(), _pollfds.size(), 10000);
		if (g_signal_status != 0)
		{
			if (g_signal_status == SIGINT || g_signal_status == SIGTERM || g_signal_status == SIGHUP)
			{
				for (size_t i = 0; i < _pollfds.size(); i++)
				{
					closeClient(i);
				}

				_pollfds.clear();
				_clientSockets.clear();

				std::cout << "[ServerManager] Server chiuso correttamente." << std::endl;
				sleep(2);
				return ;
			}
			g_signal_status = 0;
		}

        std::cout << GREEN << "[ServerManager] Listening sockets attive: " << _activeLs << RESET << std::endl;

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

            else if (_pollfds[i].revents == 0 &&_clientSockets[_pollfds[i].fd]->getLastActivity() + 30 < time(NULL))
            {
                std::cout << YELLOW << "[ServerManager] Client timeout, " << RESET;
                closeClient(i);
            }

            else if (_pollfds[i].revents & POLLIN)
                readClient(i);

            else if (_pollfds[i].revents & POLLOUT)
                writeClient(i);
        }
    }
}

Server *ServerManager::findServerByHost(const std::string& host, Server *currentServer)
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