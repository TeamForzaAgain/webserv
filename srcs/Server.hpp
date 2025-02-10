#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, ServerConfig const &serverConfig);
		~Server() {};

		std::string getServerName() const { return _hostName; }
		Server const *getServer() const { return this; }
		ListeningSocket *getListeningSocket() const { return _ls; }
		bool operator<(const Server &other) const { return _hostName < other._hostName; }

		std::string genResponse(HttpRequest const &request) const;
		HttpResponse genGetResponse(HttpRequest const &request, Location const &location) const;
		HttpResponse genPostResponse(HttpRequest const &request, Location const &location) const;
		Location findLocation(HttpRequest const &request) const;
		std::string buildFilePath(HttpRequest const &request, Location const &location) const;
		HttpResponse genErrorPage(Location const &location, int code, std::string const &message) const;
		HttpResponse genDirListing(std::string const &path, Location const &location) const;

	private:
		ListeningSocket				*_ls;
		std::string					_hostName;
		std::string					_root;
		std::vector<std::string>	_defIndexFiles;
		bool 						_defDirListing;
		std::map<int, std::string>	_errorPages;
		std::vector<Location>		_locations;

};

#endif

