#include "Server.hpp"

Server::Server(int domain, int type, int protocol, int port, u_long interface)
{
	_listeningSocket = new ListeningSocket(domain, type, protocol, port, interface);
}

Server::~Server()
{
	delete _listeningSocket;
}

static int stringToInt(const std::string &str)
{
    std::istringstream iss(str);
    int result;
    iss >> result;

    if (iss.fail()) {
        // Gestisci errori di conversione
        throw std::runtime_error("Errore: stringa non convertibile in intero");
    }

    return result;
}

int Server::parseMessage(std::string const &buffer)
{
    // Cerca la fine degli header HTTP: \r\n\r\n
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return 0; // Header incompleto
    }

    // Verifica la presenza dell'header Content-Length
    size_t contentLengthPos = buffer.find("Content-Length: ");
    if (contentLengthPos != std::string::npos) {
        // Trova il valore di Content-Length
        contentLengthPos += 16; // Salta "Content-Length: "
        size_t contentLengthEnd = buffer.find("\r\n", contentLengthPos);
        if (contentLengthEnd == std::string::npos) {
            return -1; // Header malformato
        }

     int contentLength = stringToInt(buffer.substr(contentLengthPos, contentLengthEnd - contentLengthPos));

        // Calcola la posizione di inizio del corpo del messaggio
        size_t bodyStart = headerEnd + 4; // Dopo \r\n\r\n
        size_t bodyLength = buffer.size() - bodyStart;

        // Controlla se il corpo del messaggio è completo
        if (bodyLength >= (size_t)contentLength) {
            return 1; // Messaggio completo
        } else {
            return 0; // Corpo incompleto
        }
    }

    // Nessun Content-Length: considera il messaggio completo se ha solo header
    return 1;
}


void Server::start()
{
	_listeningSocket->bindSocket();
	_listeningSocket->listenSocket();

	int activity;
	_pollfds.clear();
	struct pollfd new_pollfd;
	new_pollfd.fd = _listeningSocket->getSocketFd();
	new_pollfd.events = POLLIN;
	new_pollfd.revents = 0;
	_pollfds.push_back(new_pollfd);

	while (true)
	{
		activity = poll(_pollfds.data(), _pollfds.size(), -1);
		if (activity == -1)
		{
			perror(strerror(errno));
			throw ServerException();
		}
		if (_pollfds[0].revents & POLLIN)
		{
			int newSocket = accept(_listeningSocket->getSocketFd(), NULL, NULL);
			if (newSocket == -1)
			{
				perror(strerror(errno));
				throw ServerException();
			}
			struct pollfd client_pollfd;
			client_pollfd.fd = newSocket;
			client_pollfd.events = POLLIN;
			client_pollfd.revents = 0;
			_pollfds.push_back(client_pollfd);
		}
		for (size_t i = 1; i < _pollfds.size(); i++)
		{
			if (_pollfds[i].revents & POLLIN)
			{
				char tempBuffer[1024];
            	int bytesRead = recv(_pollfds[i].fd, tempBuffer, sizeof(tempBuffer) - 1, 0);
				if (bytesRead == -1)
				{
					perror(strerror(errno));
					throw ServerException();
				}
				 tempBuffer[bytesRead] = '\0';
                _buffers[_pollfds[i].fd] += tempBuffer;

    			if (parseMessage(_buffers[_pollfds[i].fd]))
                    _pollfds[i].events = POLLOUT;
            }
			if (_pollfds[i].revents & POLLOUT)
			{
				const char *message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";
				int bytesSent = send(_pollfds[i].fd, message, strlen(message), 0);
				if (bytesSent == -1)
				{
					perror(strerror(errno));
					throw ServerException();
				}
				close(_pollfds[i].fd);
				_pollfds.erase(_pollfds.begin() + i);
			}
		}
				
	}
}

const char *Server::ServerException::what() const throw()
{
	return "In ServerException";
}

bool Server::operator<(const Server &other) const
{
    // Puoi decidere come ordinare i server. Ecco un esempio basato sul nome del server:
    return _serverName < other._serverName;
}
