/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 23:39:08 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/07/28 00:13:54 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

#define INVALID_SOCKET				-1
#define MULTICAST_PORT  			12345
#define MULTICAST_SSRV_PORT 	12346   // For SSRV messages
#define MULTICAST_TEST_PORT  	12347   // For TEST messages
#define MULTICAST_IP    			"239.0.0.1"
#define MULTICAST_SSRV_IP    	"239.1.1.1"
#define MULTICAST_TEST_IP    	"239.1.1.0"

#include "test.hpp"

#include <cstdint>

void die(const char* message);
udp_data_t	create_sender_socket(const char* multicast_ip = MULTICAST_IP, uint16_t multicast_port = MULTICAST_PORT);
udp_data_t	create_receive_socket(const char* multicast_ip = MULTICAST_IP, uint16_t multicast_port = MULTICAST_PORT);
template <typename send_t>
void	send_udp(const udp_data_t& udp_data, send_t& data);
template <typename receive_t>
void 	receive_udp(const udp_data_t& udp_data, receive_t& data);


template <typename send_t>
void	send_udp(const udp_data_t& udp_data, send_t& data)
{
	if (sendto(udp_data.sock_fd, &data, sizeof(data), 0, (struct sockaddr*)&(udp_data.remote_addr), sizeof(udp_data.remote_addr)) < 0)
		die("sendto failed");
}

template <typename receive_t>
void 	receive_udp(const udp_data_t& udp_data, receive_t& data)
{
	static socklen_t addr_len = sizeof(udp_data.remote_addr);
	
	if (recvfrom(udp_data.sock_fd, &data, sizeof(data), 0, (struct sockaddr*)&(udp_data.remote_addr), &addr_len) < 0)
		die("recvfrom failed");
}

#endif