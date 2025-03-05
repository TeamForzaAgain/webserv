#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int type, int protocol, int port, u_long interface) :
	Socket(socket(domain, type, protocol)), _port(port), _interface(interface)
{
	if (_fd == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}

	int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("Errore nell'impostazione di SO_REUSEADDR");
        close(_fd);
        throw ListeningSocketException();
    }

	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = interface; // interface is already in network byte order by me, the boss of parsing

	// bind socket
	if (bind(_fd, (struct sockaddr*)&_address, sizeof(_address)) == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}
	// listen socket
	if (listen(_fd, 10) == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}

	setNonBlockingFd();
}

ListeningSocket::~ListeningSocket()
{
	std::cout <<MAGENTA<< "Distruttore ListeningSocket" <<RESET<< std::endl;
	close(_fd);
}

int ListeningSocket::getFd() const
{
	return _fd;
}

int ListeningSocket::getPort() const
{
	return _port;
}

u_long ListeningSocket::getInterface() const
{
	return _interface;
}

const char *ListeningSocket::ListeningSocketException::what() const throw()
{
	return "In SocketException";
}