#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Include.hpp"
#include "Server.hpp"
#include "ListeningSocket.hpp"
#include "ClientSocket.hpp"

class ServerManager
{
	public:
		ServerManager();
		~ServerManager();

		void newServer(int domain, int type, int protocol, int port, u_long interface, const std::string& serverName = "default");
		void newClient(int fd);
		void addPollFd(int fd);

		int parseMessage(std::string const &buffer);

		void run();

		class ServerManagerException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:

		std::vector<struct pollfd> _pollfds;
		std::map<Server, ListeningSocket *> _servers;
		int _activeLs;
		std::map<int, ClientSocket *> _clientSockets;

};

#endif
