#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "Include.hpp"


class Socket
{
	public:
		Socket(int domain, int type, int protocol, int port, u_long interface);
		~Socket();

		void bindSocket();
		void listenSocket();

		int getSocketFd();

		class SocketException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:
		int _socket_fd;
		struct sockaddr_in _address;
	
	
};

#endif