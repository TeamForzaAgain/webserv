#ifndef SERVER_HPP
#define SERVER_HPP

#include "Include.hpp"
#include "ListeningSocket.hpp"
#include "SessionManager.hpp"

class Server
{
	public:
		Server(ListeningSocket *ls, ServerConfig const &serverConfig);
		~Server() {};

		std::string getServerName() const { return _hostName; }
		Server const *getServer() const { return this; }
		ListeningSocket *getListeningSocket() const { return _ls; }
		int getMaxBodySize() const { return _maxBodySize; }
		bool operator<(const Server &other) const { return _hostName < other._hostName; }
		Location const *getUploadLocation() const;

		std::string genResponse(HttpRequest &request, int statusCode);
		HttpResponse genGetResponse(HttpRequest const &request, Location const &location) const;
		HttpResponse genPostResponse(HttpRequest const &request, Location const &location) const;
		HttpResponse genDeleteResponse(HttpRequest const &request, Location const &location) const;
		HttpResponse genErrorPage(Location const &location, int code) const;
		HttpResponse genAutoIndex(std::string const &path, Location const &location) const;

		HttpResponse execCgi(std::string const &targetPath, HttpRequest const &request) const;
		
		// Utility
		Location findLocation(HttpRequest const &request) const;
		std::string buildFilePath(std::string const &path, Location const &location) const;
		std::string joinPaths(const std::string& root, const std::string& path) const;
		std::string readFileContent(HttpResponse &response, std::string const &filePath) const;
		std::string findIndexFileContent(const std::string &directory, const std::vector<std::string> &indexFiles) const;
		std::string urlDecodeOnce(const std::string &str) const;

		void printServer() const;
	private:
		ListeningSocket				*_ls;
		std::string					_hostName;
		std::string					_root;
		std::vector<std::string>	_defIndexFiles;
		bool 						_defAutoIndex;
		std::map<int, std::string>	_errorPages;
		std::vector<Location>		_locations;
		int 						_maxBodySize;
		ReturnConfig				_defaultReturn;
		SessionManager				sessionManager;

};

#endif

