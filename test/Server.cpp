#include "Server.hpp"

Server::Server(int domain, int type, int protocol, int port, u_long interface)
{
	_socket = new Socket(domain, type, protocol, port, interface);
}

Server::~Server()
{
	delete _socket;
}

void Server::start()
{
	_socket->bindSocket();
	_socket->listenSocket();

	while (true)
	{
		struct sockaddr_in client_address;
		socklen_t client_address_size = sizeof(client_address);
		int client_socket_fd = accept(_socket->getSocketFd(), (struct sockaddr*)&client_address, &client_address_size);
		if (client_socket_fd == -1)
		{
			perror(strerror(errno));
			throw Server::ServerException();
		}

		std::cout << "Accepted client connection" << std::endl;
		// read the client message
		char buffer[1024];
		int bytes_read = read(client_socket_fd, buffer, sizeof(buffer));
		if (bytes_read > 0)
    		buffer[bytes_read] = '\0'; // Aggiungi terminatore
    	std::cout << "Received: " << buffer << std::endl;
		if (bytes_read == -1)
		{
			perror(strerror(errno));
			throw Server::ServerException();
		}

		close(client_socket_fd);
	}
}

const char *Server::ServerException::what() const throw()
{
	return "In ServerException";
}