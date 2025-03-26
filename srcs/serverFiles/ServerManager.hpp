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
							ServerConfig const &serverConfig);
		void newClient(int fd, Server *server);
		void addPollFd(int fd);
		Server *findServerByHost(const std::string& host, Server *currentServer);

		void run();

		class ServerManagerException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		void printConfig() const;
	private:

		std::vector<struct pollfd> _pollfds;
		std::vector<Server> _servers;
		std::vector<ListeningSocket *> _listeningSockets;
		size_t _activeLs;
		std::map<int, ClientSocket *> _clientSockets;

		void registerClient(size_t i);
		void closeClient(size_t &i);
		void readClient(size_t &i);
		void writeClient(size_t &i);


};

#endif
