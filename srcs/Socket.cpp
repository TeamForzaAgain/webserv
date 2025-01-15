/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tpicchio <tpicchio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:42:32 by tpicchio          #+#    #+#             */
/*   Updated: 2025/01/09 14:45:18 by tpicchio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(int domain, int service, int protocol, int port, unsigned long interface)
{
	this->_addr.sin_family = domain;
	this->_addr.sin_port = htons(port);
	this->_addr.sin_addr.s_addr = htonl(interface);
	this->_sock_fd = socket(domain, service, protocol);
	test_connection(this->_sock_fd);
	// conn_fd = connect_to_network(sock_fd, addr);
	// test_connection(conn_fd);
}

void Socket::test_connection(int fd)
{
	if (fd < 0)
	{
		std::cerr << "Connection failed" << std::endl;
		exit(EXIT_FAILURE);
	}
}

struct sockaddr_in Socket::get_addr()
{
	return this->_addr;
}

int Socket::get_conn_fd()
{
	return this->_conn_fd;
}

int Socket::get_sock_fd()
{
	return this->_sock_fd;
}

