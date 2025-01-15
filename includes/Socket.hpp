/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tpicchio <tpicchio@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:36:02 by tpicchio          #+#    #+#             */
/*   Updated: 2025/01/09 14:42:17 by tpicchio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Socket {
	private:
		struct sockaddr_in _addr;
		int _conn_fd;
		int _sock_fd;

	public:
		Socket(int domain, int service, int protocol, int port, unsigned long interface);
		
		virtual int connect_to_network(int sock_fd, struct sockaddr_in addr) = 0;
		
		// Getters		
		struct sockaddr_in get_addr();
		int get_conn_fd();
		int get_sock_fd();

		void test_connection(int fd);
};

#endif
