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
                                const std::string& serverName = "default")
{
	for (std::map<Server, ListeningSocket *>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        ListeningSocket *ls = it->second;

        // Caso 1: La socket ha lo stesso indirizzo e porta o esiste già una socket che ascolta su INADDR_ANY
        if (ls->getPort() == port && (ls->getInterface() == interface || ls->getInterface() == INADDR_ANY))
        {
            _servers.insert(std::make_pair(Server(ls, serverName), ls));
            return;
        }
    }

    // Se non esiste una socket compatibile, creane una nuova
    ListeningSocket *newLs = new ListeningSocket(domain, type, protocol, port, interface);
    _activeLs++;
    _servers.insert(std::make_pair(Server(newLs, serverName), newLs));

    // Aggiungi la nuova ListeningSocket al poll
    addPollFd(newLs->getFd());

    std::cout << "Nuovo server creato su IP " << interface << ", porta " << port << "." << std::endl;
}

void ServerManager::newClient(int fd)
{
    ClientSocket *newClient = new ClientSocket(fd);
    _clientSockets.insert(std::make_pair(fd, newClient));

    // Aggiungi il nuovo client al poll
    addPollFd(fd);

    std::cout << "Nuovo client connesso: " << fd << std::endl;
}


void ServerManager::run()
{
    while (true)
    {
        int activity = poll(_pollfds.data(), _pollfds.size(), -1);
        if (activity == -1)
        {
            perror(strerror(errno));
            throw ServerManagerException();
        }
        for (int i = 0; i < _activeLs; i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                int newSocket = accept(_pollfds[i].fd, NULL, NULL);
                if (newSocket == -1)
                {
                    perror(strerror(errno));
                    throw ServerManagerException();
                }
                newClient(newSocket);
                addPollFd(newSocket);
            }
        }
        for (size_t i = _activeLs; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                char tempBuffer[1024];
                int bytesRead = recv(_pollfds[i].fd, tempBuffer, sizeof(tempBuffer) - 1, 0);
                if (bytesRead == -1)
                {
                    perror(strerror(errno));
                    throw ServerManagerException();
                }
                tempBuffer[bytesRead] = '\0';
                _clientSockets[_pollfds[i].fd]->addBuffer(tempBuffer);

                if (_clientSockets[_pollfds[i].fd]->parseMessage())
                    _pollfds[i].events = POLLOUT;
            }
            if (_pollfds[i].revents & POLLOUT)
            {
                std::string response = _clientSockets[_pollfds[i].fd]->getResponse();
                send(_pollfds[i].fd, response.c_str(), response.size(), 0);
                _pollfds[i].events = POLLIN;
            }
        }
    }
}

const char *ServerManager::ServerManagerException::what() const throw()
{
    return "In ServerManagerException";
}