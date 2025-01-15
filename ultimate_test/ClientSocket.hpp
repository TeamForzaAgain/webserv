#ifndef CLIENTSOCKET_HPP
#define CLIENTSOSOCKET_HPP

#include "Include.hpp"

class ClientSocket
{
	public:
		ClientSocket(int fd);
		~ClientSocket();

		int getFd() const;
		void addBuffer(const char *buffer);

		int parseMessage();
		std::string getResponse() const;

	private:
		int _fd;
		std::vector<char> _buffer;
		std::string _response;
		bool _requestComplete;
};

#endif