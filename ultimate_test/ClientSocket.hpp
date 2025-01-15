#ifndef CLIENTSOCKET_HPP
#define CLIENTSOSOCKET_HPP

#include "Include.hpp"
#include "Server.hpp"

class ClientSocket
{
	public:
		ClientSocket(int fd, Server const *server);
		~ClientSocket();

		int getFd() const;
		void addBuffer(const char *buffer);

		int parseMessage();
		void genResponse();
		std::string getBuffer() const;
		std::string getResponse() const;

	private:
		int _fd;
		std::vector<char> _buffer;
		std::string _response;
		Server const *_server;
};

#endif