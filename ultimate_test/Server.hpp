#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, std::string const &serverName);
		~Server();

	private:
		ListeningSocket *_ls;
		std::string _serverName;
};

#endif

