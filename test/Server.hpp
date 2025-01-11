#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "Socket.hpp"

class Server
{
	public:
		Server(int domain, int type, int protocol, int port, u_long interface);
		~Server();

		void start();

		class ServerException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:
		Socket *_socket;
};

#endif