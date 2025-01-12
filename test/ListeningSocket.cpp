#include "ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int type, int protocol, int port, u_long interface)
{
	_socket_fd = socket(domain, type, protocol);
	if (_socket_fd == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}

	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);
}


ListeningSocket::~ListeningSocket()
{
	close(_socket_fd);
}

int ListeningSocket::getSocketFd()
{
	return _socket_fd;
}

void ListeningSocket::bindSocket()
{
	if (bind(_socket_fd, (struct sockaddr*)&_address, sizeof(_address)) == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}
}

void ListeningSocket::listenSocket()
{
	if (listen(_socket_fd, 5) == -1)
	{
		perror(strerror(errno));
		throw ListeningSocketException();
	}
}

const char *ListeningSocket::ListeningSocketException::what() const throw()
{
	return "In SocketException";
}
