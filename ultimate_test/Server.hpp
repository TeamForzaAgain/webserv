#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, ServerConfig const &serverConfig);
		~Server();
		std::string genResponse(std::string const &request) const;

		std::string getServerName() const;
		Server const *getServer() const;
		ListeningSocket *getListeningSocket() const;
		bool operator<(const Server &other) const;

	private:
		ListeningSocket *_ls;
		std::string _serverName;
		Route _defaultRoute;
		std::vector<Route> _routes;
		std::map<int, std::string> _errorPages;
		
};

#endif

