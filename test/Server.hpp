#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(int domain, int type, int protocol, int port, u_long interface);
		~Server();
		bool operator<(const Server &other) const;

		void start();
		int parseMessage(std::string const &buffer);

		class ServerException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:
		ListeningSocket *_listeningSocket;
		std::vector<struct pollfd> _pollfds;
		std::map<int, std::string> _buffers;
};

#endif