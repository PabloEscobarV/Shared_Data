/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 10:11:16 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/17 13:35:06 by blackrider       ###   ########.fr       */
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

#define SEND_DURATION		50000 // Duration in milliseconds for sending messages
#define SSV_SLEEP_TIME	300
#define TIME_20_MS			20 // Timeout in milliseconds for SSV messages
#define INVALID_SOCKET	-1
#define MULTICAST_PORT  12345
#define MULTICAST_IP    "239.0.0.1"
#define TICK_PERIOD			10 // Period in milliseconds for the tick counter

using namespace std;

std::mutex mtx_out;

ParamData<P_COUNT>	*param_data;
ParamData<P_COUNT>	*old_param_data;
SharedData<P_COUNT> *shared_data;

struct	udp_data_t
{
	int 				sock_fd;
	sockaddr_in remote_addr;
};

void die(const char* message) 
{
  perror(message);
  exit(1);
}

void	send(udp_data_t& udp_data, uint16_t ssrv_freq = 0,  SharedData<P_COUNT> *shared_data = nullptr, ParamData<P_COUNT> *param_data = nullptr)
{
	can_data_t	can_data {};

	can_data.idx_can = getpid();
	mtx_out.lock();
	cout << "ssrv_freq: " << ssrv_freq << endl;
	mtx_out.unlock();
	for (int i = 0; i < SEND_DURATION; ++i)
	{
		shared_data->period_counter();
		// if (i != 0 && ssrv_freq != 0 && !(ssrv_freq % (i)))
		// 	shared_data->add_ssrv_message(param_data->get_param_num(rand() % (P_COUNT)), rand() % 1000);
		if (shared_data->get_messages(can_data))
		{
			if (sendto(udp_data.sock_fd, &can_data, sizeof(can_data), 0, (struct sockaddr*)&(udp_data.remote_addr), sizeof(udp_data.remote_addr)) < 0)
				die("sendto failed");
		}
		this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
	}
}

udp_data_t	sender_socket()
{
	int	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in	remote_addr {};
	udp_data_t udp_data {};

	if (sock_fd == INVALID_SOCKET)
		die("INVALID SENDER SOCKET!!!");
	remote_addr.sin_addr.s_addr = inet_addr(MULTICAST_IP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(MULTICAST_PORT);
	udp_data.sock_fd = sock_fd;
	udp_data.remote_addr = remote_addr;
	return udp_data;
}

void	check_param_data(ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		if (param_data->get_param_value(i) != old_param_data->get_param_value(i))
		{
			mtx_out.lock();
			cout << "Param " << param_data->get_param_num(i) << " changed from "
					 << old_param_data->get_param_value(i) << " to "
					 << param_data->get_param_value(i) << endl;
			mtx_out.unlock();
			old_param_data->set_param_value(i, param_data->get_param_num(i), param_data->get_param_value(i));
		}
	}
}

void	receiver_ssv(udp_data_t& udp_data,
									SharedData<P_COUNT> *shared_data,
									ParamData<P_COUNT> *param_data, 
									ParamData<P_COUNT> *old_param_data)
{
	socklen_t addr_len = sizeof(udp_data.remote_addr);
	can_data_t	can_data {};

  while (true)
  {
    int n = recvfrom(udp_data.sock_fd, &can_data, sizeof(can_data), 0, (struct sockaddr*)&(udp_data.remote_addr), &addr_len);
    if (n < 0)
      die("recvfrom failed");
    if (can_data.idx_can != getpid())
    {
			can_data.idx = getpid();
      shared_data->handle_messages(can_data);
			check_param_data(param_data, old_param_data);
    }
  }
}

udp_data_t	create_receive_socket()
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
	addr.sin_port = htons(MULTICAST_PORT);
	if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		die("bind failed");
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
		die("setsockopt(IP_ADD_MEMBERSHIP) failed");
	udp_data.sock_fd = socket_fd;
	udp_data.remote_addr = remote_addr;
	return udp_data;
}

void	crt_trhreads(uint16_t ssrv_freq, SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
	thread receiver_thread([&]()
	{
		mtx_out.lock();
		cout << "Receiver thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_receiver = create_receive_socket();
		receiver_ssv(udp_data_receiver, shared_data, param_data, old_param_data);
		close(udp_data_receiver.sock_fd);
	});
	// thread sender_thread([&]()
	// {
	mtx_out.lock();
	cout << "Sender thread started." << endl;
	mtx_out.unlock();
	udp_data_t udp_data_sender = sender_socket();
	send(udp_data_sender, ssrv_freq, shared_data, param_data);
	close(udp_data_sender.sock_fd);
	exit(0);
	// });
	// sender_thread.join();
	receiver_thread.join();
}

void	init_param_data(ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data, uint16_t step_kef)
{
	int32_t param_value = 0;

	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		param_value = static_cast<int32_t>(std::rand() % 9999 + 2077);
		param_data->set_param_value(i, i * step_kef, param_value);
		old_param_data->set_param_value(i, i * step_kef, param_value);
	}
}

void	print_param_data(ParamData<P_COUNT> *param_data)
{
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		std::cout << "Param " << param_data->get_param_num(i) << ": " << param_data->get_param_value(i) << std::endl;
	}
}

void	init_shared_data(SharedData<P_COUNT> *shared_data,
											ParamData<P_COUNT> *param_data,
											int16_t i_start_value = 0)
{
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		shared_data->set_param_num(param_data->get_param_num(i));
		shared_data->set_iterator(i, i_start_value);
	}
}

int main(int argc, char ** argv)
{
	int16_t iterator_start_value = 0;
	uint16_t param_kef = 0;
	uint16_t ssrv_freq = 0;
	param_data = new ParamData<P_COUNT>();
	old_param_data = new ParamData<P_COUNT>();
	shared_data = new SharedData<P_COUNT>();
	srand(time(NULL) + getpid()); // Seed random number generator with current time and process ID
	
	if (argc > 1)
		iterator_start_value = atoi(argv[1]);
	if (argc > 2)
		param_kef = atoi(argv[2]);
	if (argc > 3)
		ssrv_freq = atoi(argv[3]);
	cout << "PID: " << getpid() << endl;
	if (argc < 2)
	{
		cout << "Enter iterator start value: \n";
		cin >> iterator_start_value;
		cin.get();
	}
	if (argc < 3)
	{
		cout << "Enter parameter step kef: \n";
		cin >> param_kef;
		cin.get();
	}
	if (argc < 4)
	{
		cout << "Enter ssrv frequency (0 for no ssrv messages): \n";
		cin >> ssrv_freq;
	}
	init_param_data(param_data, old_param_data,  param_kef);
	init_shared_data(shared_data, param_data, iterator_start_value);
	print_param_data(param_data);
	crt_trhreads(ssrv_freq, shared_data, param_data, old_param_data);
	delete param_data;
	delete shared_data;
}
