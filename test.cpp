/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 10:11:16 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/13 14:50:38 by Pablo Escob      ###   ########.fr       */
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

#define SEND_DURATION		1000 // Duration in milliseconds for sending messages
#define SSV_SLEEP_TIME	100
#define TIME_20_MS			20 // Timeout in milliseconds for SSV messages
#define INVALID_SOCKET	-1
#define MULTICAST_PORT  12345
#define MULTICAST_IP    "239.0.0.1"

using namespace std;

struct	udp_data_t
{
	uint16_t		idx;
	can_data_t	can_data;
};

void die(const char* message) 
{
  perror(message);
  exit(1);
}

void	sender_socket(SharedData<P_COUNT> *shared_data)
{
	int	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in	remote_addr {};
	can_data_t	can_data {};

	if (sock_fd == INVALID_SOCKET)
		die("INVALID SENDER SOCKET!!!");
	remote_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(MULTICAST_PORT);
	can_data.idx_can = getpid();
	for (int i = 0; i < SEND_DURATION; ++i)
	{
		shared_data->period_counter();
		if (shared_data->get_messages(can_data))
		{
			if (sendto(sock_fd, &can_data, sizeof(can_data), 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0)
				die("sendto failed");
		}
		this_thread::sleep_for(chrono::milliseconds(20));
	}
}

void	receiver_ssv(int socket_fd, sockaddr_in remote_addr, SharedData<P_COUNT> *shared_data)
{
	socklen_t addr_len = sizeof(remote_addr);
	can_data_t	can_data {};

  while (true)
  {
    int n = recvfrom(socket_fd, &can_data, sizeof(can_data), 0, (struct sockaddr*)&remote_addr, &addr_len);
    if (n < 0)
      die("recvfrom failed");
    if (can_data.idx != getpid())
    {
      shared_data->handle_messages(can_data);
    }
  }
}

void	create_receive_socket(SharedData<P_COUNT> *shared_data)
{
	int reuse = 1;
	int	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in	addr {};
	struct ip_mreq			mreq {};
	struct sockaddr_in	remote_addr {};

	if (socket_fd == INVALID_SOCKET)
		die("INVALID SOCKET");
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)(&reuse), sizeof(reuse)))
		die("setsockopt(SO_REUSEADDR) failed");
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(MULTICAST_PORT);
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("bind failed");
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
		die("setsockopt(IP_ADD_MEMBERSHIP) failed");
	receiver_ssv(socket_fd, remote_addr, shared_data);
}

void	init_param_data(ParamData<P_COUNT> *param_data, uint16_t step_kef)
{
	int32_t param_value = 0;

	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		param_value = static_cast<int32_t>(std::rand() % 9999 + 2077);
		param_data->set_param_value(i, i * step_kef, param_value);
	}
}

void	print_param_data(ParamData<P_COUNT> *param_data)
{
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		std::cout << "Param " << param_data->get_param_num(i) << ": " << param_data->get_param_value(i) << std::endl;
	}
}

int main()
{
	param_data = new ParamData<P_COUNT>();
	shared_data = new SharedData<P_COUNT>();
	
	init_param_data(param_data, 2);
	print_param_data(param_data);

	delete param_data;
	delete shared_data;
}
