#ifndef CLIENTSOCKET_HPP
#define CLIENTSOSOCKET_HPP

#include "Include.hpp"
#include "Server.hpp"
#include "Socket.hpp"


class ServerManager;

class ClientSocket : public Socket
{
	public:
		ClientSocket(int fd, Server const *server);
		~ClientSocket();

		int getFd() const;
		void addBuffer(const char *buffer);

		int parseEndMessage();
		void parseHostConnection(ServerManager &serverManager, const std::string &request);
		void genResponse(ServerManager &serverMenager);
		std::string getBuffer() const;
		std::string getResponse() const;
		bool getKeepAlive() const;

	private:
		int _fd;
		std::vector<char> _buffer;
		std::string _response;
		Server const *_server;
		bool _keepAlive;

};

#endif