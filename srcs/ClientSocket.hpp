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

		void addBuffer(const char *buffer, int bytesRead);
		int parseRequest(ServerManager &serverManager);
		void genResponse(ServerManager &serverMenager);
		void setLastActivity();

		time_t getLastActivity() const;
		int getFd() const;
		std::string getBuffer() const;
		std::string getResponse() const;
		int getStatus() const;
		bool getKeepAlive() const;

	private:
		int _fd;
		Server const *_server;
		int _status;
		std::vector<char> _buffer;
		HttpRequest _request;
		std::string _response;
		time_t _lastActivity;
		bool _keepAlive;
		int _contentLength;
		int _chunkLength;
		int _headersLenght;
		bool _toUpload;
		std::string _boundary;
		std::ofstream _uploadFile;
		int _bytesWritten;

};

#endif