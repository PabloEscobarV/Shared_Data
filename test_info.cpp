/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_info.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 07:01:18 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/22 07:46:26 by blackrider       ###   ########.fr       */
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
#define MULTICAST_TEST_PORT  12347
#define MULTICAST_TEST_IP    "239.1.1.0"

void die(const char* message) 
{
  perror(message);
  exit(1);
}

udp_data_t	create_receive_socket(const char* multicast_ip = MULTICAST_TEST_IP, uint16_t multicast_port = MULTICAST_TEST_PORT)
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

void	handle_test_information(test_data_t	**test_info, test_data_t& test_data)
{
	while (test_info)
	{
		count_test_info(*test_info);
		++test_info;
	}
}

void	receive_start_message(test_data_t	**test_info, const udp_data_t& udp_data)
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

test_data_t	**create_test_info_array(uint16_t count)
{
	test_data_t	**test_info = new test_data_t *[count + 1] {};
	for (int i = 0; i < count; ++i)
	{
		test_info[i] = new test_data_t [P_COUNT];
	}
	test_info[count] = nullptr;
	return test_info;
}

int main()
{
	uint16_t	count;
	udp_data_t udp_data_receiver = create_receive_socket();
	test_data_t	**test_info;
	cout << "Enter the number of proccess to receive: ";
	cin >> count;
	test_info = create_test_info_array(count);
	receive_start_message(udp_data_receiver);
	close(udp_data_receiver.sock_fd);
	for (int i = 0; i < count; ++i)
	{
		delete[] test_info[i];
	}
	delete[] test_info;
	return 0;
}
