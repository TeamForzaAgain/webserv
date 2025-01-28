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

		void newServer(int domain, int type, int protocol, int port, u_long interface,
							const std::string& serverName, std::string const &htmlPath);
		void newClient(int fd, Server const *server);
		void addPollFd(int fd);
		Server const *findServerByHost(const std::string& host, Server const *currentServer);

		void run();

		class ServerManagerException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

	private:

		std::vector<struct pollfd> _pollfds;
		std::map<Server, ListeningSocket *> _servers;
		size_t _activeLs;
		std::map<int, ClientSocket *> _clientSockets;

		void registerClient(size_t i);
		void closeClient(size_t &i);
		void readClient(size_t &i);
		void writeClient(size_t &i);

};

#endif
