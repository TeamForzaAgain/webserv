#include "Socket.hpp"

Socket::Socket(int fd) : _fd(fd)
{}

Socket::~Socket()
{
	close(_fd);
}

int Socket::getFd() const
{
	return _fd;
}

void Socket::setNonBlockingFd()
{
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl error");
        throw SocketException();
    }

    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl error");
        throw SocketException();
    }
}

const char *Socket::SocketException::what() const throw()
{
	return "In SocketException";
}