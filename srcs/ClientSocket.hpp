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
		int genResponse(ServerManager &serverMenager);
		std::string getBuffer() const;
		std::string getResponse() const;
		bool getKeepAlive() const;

	private:
		int _fd;
		std::vector<char> _buffer;
		HttpRequest _request;
		std::string _response;
		Server const *_server;
		bool _keepAlive;

};

#endif