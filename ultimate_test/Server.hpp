#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, ServerConfig const &serverConfig);
		~Server() {};

		std::string getServerName() const { return _serverName; }
		Server const *getServer() const { return this; }
		ListeningSocket *getListeningSocket() const { return _ls; }
		bool operator<(const Server &other) const { return _serverName < other._serverName; }

		std::string genResponse(std::string const &request) const;
		std::string buildFilePath(std::string const &request) const;

	private:
		ListeningSocket				*_ls;
		std::string					_serverName;
		std::string					_defRoute;
		std::vector<std::string>	_defIndexes;
		bool 						_defDirListing;
		std::vector<Route>			_routes;

};

#endif

