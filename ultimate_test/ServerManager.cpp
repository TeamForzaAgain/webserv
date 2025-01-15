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
            return;
        }
    }

    // Se non esiste una socket compatibile, creane una nuova
    ListeningSocket *newLs = new ListeningSocket(domain, type, protocol, port, interface);
    _activeLs++;
    _servers.insert(std::make_pair(Server(newLs, serverName, htmlPath), newLs));

    // Aggiungi la nuova ListeningSocket al poll
    addPollFd(newLs->getFd());

    std::cout << "Nuovo server creato su IP " << interface << ", porta " << port << "." << std::endl;
}

void ServerManager::newClient(int fd, Server const *server)
{
    ClientSocket *newClient = new ClientSocket(fd, server);
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
                // find the server that corresponds to the listening socket
                Server const *server;
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
                {   
                    std::cout << BLUE << "Request: " << _clientSockets[_pollfds[i].fd]->getBuffer() << RESET << std::endl;
                    _clientSockets[_pollfds[i].fd]->genResponse();
					std::cout << GREEN << "Response: " << _clientSockets[_pollfds[i].fd]->getResponse() << RESET << std::endl;
                    _pollfds[i].events = POLLOUT;
                }
            }
            if (_pollfds[i].revents & POLLOUT)
            {
                std::string response = _clientSockets[_pollfds[i].fd]->getResponse();
                std::cout << "Sending response: " << response << std::endl;
                int bytesSent = send(_pollfds[i].fd, response.c_str(), response.size(), 0);
				if (bytesSent == -1)
				{
					perror(strerror(errno));
					throw ServerManagerException();
				}
                // _pollfds[i].events = POLLIN;
				// _pollfds[i].revents = 0;
				close(_pollfds[i].fd);
				_pollfds.erase(_pollfds.begin() + i);
            }
        }
    }
}

const char *ServerManager::ServerManagerException::what() const throw()
{
    return "In ServerManagerException";
}