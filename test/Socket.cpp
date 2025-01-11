#include "Socket.hpp"

Socket::Socket(int domain, int type, int protocol, int port, u_long interface)
{
	_socket_fd = socket(domain, type, protocol);
	if (_socket_fd == -1)
	{
		perror(strerror(errno));
		throw SocketException();
	}

	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);
}


Socket::~Socket()
{
	close(_socket_fd);
}

int Socket::getSocketFd()
{
	return _socket_fd;
}

void Socket::bindSocket()
{
	if (bind(_socket_fd, (struct sockaddr*)&_address, sizeof(_address)) == -1)
	{
		perror(strerror(errno));
		throw SocketException();
	}
}

void Socket::listenSocket()
{
	if (listen(_socket_fd, 5) == -1)
	{
		perror(strerror(errno));
		throw SocketException();
	}
}

const char *Socket::SocketException::what() const throw()
{
	return "In SocketException";
}
