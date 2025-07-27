/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 23:37:35 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/28 00:04:37 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/socket.hpp"

#include "../hdrs/test.hpp"

#include <chrono>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>

using namespace std;

void die(const char* message) 
{
  perror(message);
  exit(1);
}

udp_data_t	create_sender_socket(const char* multicast_ip, uint16_t multicast_port)
{
	int	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in	remote_addr {};
	udp_data_t udp_data {};

	if (sock_fd == INVALID_SOCKET)
		die("INVALID SENDER SOCKET!!!");
	remote_addr.sin_addr.s_addr = inet_addr(multicast_ip);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(multicast_port);
	udp_data.sock_fd = sock_fd;
	udp_data.remote_addr = remote_addr;
	return udp_data;
}

udp_data_t	create_receive_socket(const char* multicast_ip, uint16_t multicast_port)
{
	int reuse = 1;
	int	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in	addr {};
	struct ip_mreq			mreq {};
	struct sockaddr_in	remote_addr {};
	udp_data_t udp_data {};

	if (socket_fd == INVALID_SOCKET)
		die("INVALID SOCKET");
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)(&reuse), sizeof(reuse)))
		die("setsockopt(SO_REUSEADDR) failed");
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(multicast_port);
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("bind failed");
	mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
		die("setsockopt(IP_ADD_MEMBERSHIP) failed");
	udp_data.sock_fd = socket_fd;
	udp_data.remote_addr = remote_addr;
	return udp_data;
}
