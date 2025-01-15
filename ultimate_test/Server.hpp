#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, std::string const &serverName, std::string const &htmlPath);
		~Server();
		std::string genResponse(std::string const &request) const;

		bool operator<(const Server &other) const;

	private:
		ListeningSocket *_ls;
		std::string _serverName;
		std::string _htmlPath;
};

#endif

