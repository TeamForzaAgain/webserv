#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "Include.hpp"

class Socket
{
	public:
		Socket(int fd);
		~Socket();

		int getFd() const;
		void setNonBlockingFd();

		class SocketException : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};
	
	protected:
		int _fd;
};

#endif