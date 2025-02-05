#ifndef LISTENINGSOCKET_HPP
#define LISTENINGSOCKET_HPP

#include "Include.hpp"
#include "Socket.hpp"

class ListeningSocket : public Socket
{
	public:
		ListeningSocket(int domain, int type, int protocol, int port, u_long interface);
		~ListeningSocket();

		int getFd() const;
		int getPort() const;
		u_long getInterface() const;

		class ListeningSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:
		int _port;
		u_long _interface;
		struct sockaddr_in _address;
};

#endif