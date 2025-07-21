/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_ssrv.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 06:51:11 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/21 14:50:39 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hdrs/shared_data.hpp"
#include "hdrs/client_server_shared_setpoint.hpp"
#include "hdrs/test.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <mutex>

using namespace std;

#define INVALID_SOCKET	-1
#define MULTICAST_SSRV_PORT  12346
#define MULTICAST_SSRV_IP    "239.1.1.1"

void die(const char* message) 
{
  perror(message);
  exit(1);
}

udp_data_t	sender_socket(const char* multicast_ip = MULTICAST_SSRV_IP, uint16_t multicast_port = MULTICAST_SSRV_PORT)
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

udp_data_t	create_receive_socket(const char* multicast_ip = MULTICAST_SSRV_IP, uint16_t multicast_port = MULTICAST_SSRV_PORT)
{
	int	reuse = 1;
	int	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in	addr {};
	struct ip_mreq			mreq {};
	udp_data_t udp_data {};

	if (socket_fd == INVALID_SOCKET)
		die("INVALID SOCKET");
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)(&reuse), sizeof(reuse)) < 0)
		die("setsockopt(SO_REUSEADDR) failed");
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(multicast_port);
	if (bind(socket_fd, (struct sockaddr *)(&addr), sizeof(addr)) < 0)
		die("bind failed");
	mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)(&mreq), sizeof(mreq)) < 0)
		die("setsockopt(IP_ADD_MEMBERSHIP) failed");
	udp_data.sock_fd = socket_fd;
	return udp_data;
}

ssrv_data_t	get_ssrv_data()
{
	ssrv_data_t ssrv_start_message = {};
	
	cout  << "Enter SSRV start message parameters:" << endl;
	cout << "ID: ";
	cin >> ssrv_start_message.id;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Parameter Index: ";
	cin >> ssrv_start_message.param_idx;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Parameter Value: ";
	cin >> ssrv_start_message.param_val;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return ssrv_start_message;
}

void	send_ssrv_start_message(const udp_data_t& udp_data)
{
	ssrv_data_t ssrv_start_message = {};

	while (true)
	{
		ssrv_start_message = get_ssrv_data();
		cout << "Entered ID: " << ssrv_start_message.id
			 << ", Parameter Index: " << ssrv_start_message.param_idx
			 << ", Parameter Value: " << ssrv_start_message.param_val << endl;
		if (sendto(udp_data.sock_fd, &ssrv_start_message, sizeof(ssrv_start_message), 0,
				(struct sockaddr *)(&udp_data.remote_addr), sizeof(udp_data.remote_addr)) < 0)
			die("sendto failed");
	}
}

void	count_test_info(test_data_t	*test_info)
{
	static const float	percent = 100.0 / P_COUNT;
	uint16_t	num_count = 0;
	uint16_t	value_count = 0;
	uint16_t	iterator_count = 0;

	for (int i = 0; i < P_COUNT; ++i)
	{
		if (test_info[i].param_num != test_info[0].param_num)
		{
			++num_count;
		}
		if (test_info[i].iterator != test_info[0].iterator)
		{
			++iterator_count;
		}
		if (test_info[i].param_val != test_info[0].param_val)
		{
			++value_count;
		}
	}
	cout << "Cycle: " << test_info[0].i << endl;
	cout << "Diferent parameters: " << num_count * percent << " [%]" << endl;
	cout << "Diferent iterators: " << iterator_count * percent << " [%]" << endl;
	cout << "Diferent values: " << value_count * percent << " [%]" << endl;
}

void	handle_test_information(test_data_t& test_data)
{
	static uint16_t	i = 0;
	static uint16_t	idx = test_data.i;
	static test_data_t	*test_info = new test_data_t [P_COUNT] {};
	
	if (idx == test_data.i)
	{
		test_info[i] = test_data;
		++i;
	}
	if (i >= P_COUNT)
		count_test_info(test_info);
}

void	receive_start_message(const udp_data_t& udp_data)
{
	test_data_t test_data {};
	socklen_t addr_len = sizeof(udp_data.remote_addr);
	
	while (true)
	{
		if (recvfrom(udp_data.sock_fd, &test_data, sizeof(test_data), 0,
				(struct sockaddr *)(&udp_data.remote_addr), &addr_len) < 0)
			die("recvfrom failed");
		handle_test_information(test_data);
	}
}

int	main()
{
	udp_data_t udp_data_sender = sender_socket();
	send_ssrv_start_message(udp_data_sender);
	close(udp_data_sender.sock_fd);
	return 0;
}
