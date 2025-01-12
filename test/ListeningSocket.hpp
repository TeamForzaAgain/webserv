#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Include.hpp"


class ListeningSocket
{
	public:
		ListeningSocket(int domain, int type, int protocol, int port, u_long interface);
		~ListeningSocket();

		void bindSocket();
		void listenSocket();

		int getSocketFd();

		class ListeningSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:
		int _socket_fd;
		struct sockaddr_in _address;
};

#endif