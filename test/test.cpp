/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 10:11:16 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/22 15:48:32 by blackrider       ###   ########.fr       */
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

#define SEND_DURATION					100000 // Duration in milliseconds for sending messages
#define SSV_SLEEP_TIME				100
#define TIME_20_MS						20 // Timeout in milliseconds for SSV messages
#define INVALID_SOCKET				-1
#define MULTICAST_PORT  			12345
#define MULTICAST_SSRV_PORT 	12346   // For SSRV messages
#define MULTICAST_TEST_PORT  	12347   // For TEST messages
#define MULTICAST_IP    			"239.0.0.1"
#define MULTICAST_SSRV_IP    	"239.1.1.1"
#define MULTICAST_TEST_IP    	"239.1.1.0"
#define TICK_PERIOD						20 // Period in milliseconds for the tick counter

using namespace std;

std::mutex mtx_out;
static uint16_t PID = 0;

ParamData<P_COUNT>	*param_data;
ParamData<P_COUNT>	*old_param_data;
SharedData<P_COUNT> *shared_data;

uint16_t	get_pid()
{
	return PID;
}

void die(const char* message) 
{
  perror(message);
  exit(1);
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

void	send(udp_data_t& udp_data, SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data)
{
	can_data_t	can_data {};
	uint16_t		send_ssrv_time = rand() % 50 + 1;

	can_data.idx_can = get_pid();
	mtx_out.lock();
	cout << "send_ssrv_time: " << send_ssrv_time << endl;
	mtx_out.unlock();
	for (int i = 0; i < SEND_DURATION; ++i)
	{
		shared_data->period_counter();
		// if (i != 0 && !(send_ssrv_time % (i)))
		// 	shared_data->add_ssrv_message(param_data->get_param_num(rand() % (P_COUNT)), rand() % 1000);
		if (shared_data->get_messages(can_data))
		{
			if (sendto(udp_data.sock_fd, &can_data, sizeof(can_data), 0, (struct sockaddr*)&(udp_data.remote_addr), sizeof(udp_data.remote_addr)) < 0)
				die("sendto failed");
			check_param_data(param_data, old_param_data);
		}
		this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
	}
}

void	send_test_data(udp_data_t& udp_data, SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data)
{
	test_data_t test_data {};
	test_data.pid = get_pid();
	test_data.iterator = 0;
	test_data.i = 0;
	
	while (true)
	{
		for (int i = 0; i < P_COUNT; ++i)
		{
			test_data.param_num = param_data->get_param_num(i);
			test_data.param_val = param_data->get_param_value(i);
			test_data.iterator = shared_data->get_iterator(i);
			test_data.param_idx = i;
			if (sendto(udp_data.sock_fd, &test_data, sizeof(test_data), 0, (struct sockaddr*)&(udp_data.remote_addr), sizeof(udp_data.remote_addr)) < 0)
				die("sendto failed");
		}
		++test_data.i;
		this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD * 2));
	}
}

udp_data_t	sender_socket(const char* multicast_ip = MULTICAST_IP, uint16_t multicast_port = MULTICAST_PORT)
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
    if (can_data.idx_can != get_pid())
    {
			can_data.idx = get_pid();
      shared_data->handle_messages(can_data);
			check_param_data(param_data, old_param_data);
    }
  }
}

void	receive_ssrv_request(udp_data_t& udp_data,
													SharedData<P_COUNT> *shared_data,
													ParamData<P_COUNT> *param_data)
{
	ssrv_data_t ssrv_req_message {};
	socklen_t addr_len = sizeof(udp_data.remote_addr);
	
	while (true)
	{
		recvfrom(udp_data.sock_fd, &ssrv_req_message, sizeof(ssrv_req_message), 0, (struct sockaddr*)&(udp_data.remote_addr), &addr_len);
		if (ssrv_req_message.id == get_pid())
		{
			mtx_out.lock();
			cout << "Received SSRV request for: " << ssrv_req_message.id
						<< ", Param Index: " << ssrv_req_message.param_idx
						<< ", Param Value: " << ssrv_req_message.param_val << endl;
			mtx_out.unlock();
			shared_data->add_ssrv_message(param_data->get_param_num(ssrv_req_message.param_idx), ssrv_req_message.param_val);
		}
	}
}

udp_data_t	create_receive_socket(const char* multicast_ip = MULTICAST_IP, uint16_t multicast_port = MULTICAST_PORT)
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

void	crt_trhreads(SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
	thread	send_test_data_thread([&]()
	{
		mtx_out.lock();
		cout << "Sender test data thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_sender = sender_socket(MULTICAST_TEST_IP, MULTICAST_TEST_PORT);
		send_test_data(udp_data_sender, shared_data, param_data);
		close(udp_data_sender.sock_fd);
	});
	thread sender_thread([&]()
	{
		mtx_out.lock();
		cout << "Sender thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_sender = sender_socket();
		send(udp_data_sender, shared_data, param_data);
		close(udp_data_sender.sock_fd);
		exit(0); // Exit after sending messages
	});
	thread receiver_thread([&]()
	{
		mtx_out.lock();
		cout << "Receiver thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_receiver = create_receive_socket();
		receiver_ssv(udp_data_receiver, shared_data, param_data, old_param_data);
		close(udp_data_receiver.sock_fd);
	});
	thread ssrv_receiver_thread([&]()
	{
		mtx_out.lock();
		cout << "SSRV Receiver thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_ssrv_receiver = create_receive_socket(MULTICAST_SSRV_IP, MULTICAST_SSRV_PORT);
		receive_ssrv_request(udp_data_ssrv_receiver, shared_data, param_data);
		close(udp_data_ssrv_receiver.sock_fd);
	});
	receiver_thread.join();
	ssrv_receiver_thread.join();
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

// void	start_test(uint16_t pid, uint16_t iterator_start_value, uint16_t param_kef)
// {
// 	param_data = new ParamData<P_COUNT>();
// 	old_param_data = new ParamData<P_COUNT>();
// 	shared_data = new SharedData<P_COUNT>();

// 	srand(time(NULL) + get_pid() + getpid()); // Seed random number generator with current time and process ID
// 	cout << "PID: " << get_pid() << endl;
// 	init_param_data(param_data, old_param_data,  param_kef);
// 	init_shared_data(shared_data, param_data, iterator_start_value);
// 	print_param_data(param_data);
// 	crt_trhreads(shared_data, param_data, old_param_data);
// 	delete param_data;
// 	delete shared_data;
// }

int main(int argc, char *argv[])
{
	uint16_t iterator_start_value;
	uint16_t param_kef = 0;
	param_data = new ParamData<P_COUNT>();
	old_param_data = new ParamData<P_COUNT>();
	shared_data = new SharedData<P_COUNT>();
	srand(time(NULL) + get_pid() + getpid()); // Seed random number generator with current time and process ID
	PID = getpid(); // Limit PID to 3 digits for easier reading
	if (argc < 4)
	{
		cout << "Enter process ID (PID): \n";
		cin >> PID;
		cout << "Enter iterator start value: \n";
		cin >> iterator_start_value;
		cout << "Enter parameter step kef: \n";
		cin >> param_kef;
	}
	else
	{
		PID = static_cast<uint16_t>(atoi(argv[1]));
		iterator_start_value = static_cast<uint16_t>(atoi(argv[2]));
		param_kef = static_cast<uint16_t>(atoi(argv[3]));
		cout << "PID: " << get_pid() << endl;
	}
	cout << "PID: " << get_pid() << endl;
	init_param_data(param_data, old_param_data,  param_kef);
	init_shared_data(shared_data, param_data, iterator_start_value);
	print_param_data(param_data);
	crt_trhreads(shared_data, param_data, old_param_data);
	delete param_data;
	delete shared_data;
}
