/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 10:11:16 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/30 00:00:16 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_data.hpp"
#include "../hdrs/client_server_shared_setpoint.hpp"
#include "../hdrs/test.hpp"
#include "../hdrs/socket.hpp"
#include "../hdrs/time_stampt.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <mutex>

#define SEND_DURATION		100000
#define TICK_PERIOD			20 // Period in milliseconds for the tick counter

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

bool	check_param_data(ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
	bool changed = false;
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		if (param_data->get_param_value(i) != old_param_data->get_param_value(i))
		{
			mtx_out.lock();
			cout << "Param " << param_data->get_param_num(i) << " changed from "
					 << old_param_data->get_param_value(i) << " to "
					 << param_data->get_param_value(i) << endl;
			print_time_stamp();
			mtx_out.unlock();
			old_param_data->set_param_value(i, param_data->get_param_num(i), param_data->get_param_value(i));
			changed = true;
		}
	}
	return changed;
}

void	send_ssv(udp_data_t& udp_data, SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data)
{
	can_data_t	can_data {};
	uint16_t		send_ssrv_time = rand() % 50 + 1;

	can_data.idx_can = get_pid();
	for (int i = 0; i < SEND_DURATION; ++i)
	{
		shared_data->period_counter();
		if (shared_data->get_messages(can_data))
		{
			send_udp(udp_data, can_data);
		}
		this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
	}
}

int32_t	*crt_para_arr()
{
	int32_t	*param_arr = new int32_t[P_COUNT];
	for (uint16_t i = 0; i < P_COUNT; ++i)
	{
		param_arr[i] = param_data->get_param_value(i);
	}
	return param_arr;
}

void	send_test_data(udp_data_t& udp_data,
										SharedData<P_COUNT> *shared_data,
										ParamData<P_COUNT> *param_data)
{
	test_data_t test_data {};
	test_data.pid = get_pid();
	static ParamData<P_COUNT> *old_param_data = new ParamData<P_COUNT>();
	
	while (true)
	{
		for (int i = 0; i < P_COUNT; ++i)
		{
			if (param_data->get_param_value(i) != old_param_data->get_param_value(i))
			{
				test_data.param_num = param_data->get_param_num(i);
				test_data.param_idx = i;
				test_data.iterator = shared_data->get_iterator(i);
				test_data.param_val = param_data->get_param_value(i);
				old_param_data->set_param_value(i, test_data.param_num, test_data.param_val);
				send_udp(udp_data, test_data);

				mtx_out.lock();
				cout << "Sender PID: " << test_data.pid
						 << ", Param Index: " << test_data.param_idx
						 << ", Param Number: " << test_data.param_num
						 << ", Param Value: " << test_data.param_val
						 << ", Iterator: " << test_data.iterator << endl;
				print_time_stamp();
				mtx_out.unlock();
			}
		}
		this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
	}
}

void	receiver_ssv(udp_data_t& udp_data,
									SharedData<P_COUNT> *shared_data,
									ParamData<P_COUNT> *param_data)
{
	socklen_t addr_len = sizeof(udp_data.remote_addr);
	can_data_t	can_data {};

  while (true)
  {
		receive_udp(udp_data, can_data);
    if (can_data.idx_can != get_pid())
    {
			can_data.idx = get_pid();
      shared_data->handle_messages(can_data);
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
		receive_udp(udp_data, ssrv_req_message);
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

void	crt_trhreads(SharedData<P_COUNT> *shared_data, ParamData<P_COUNT> *param_data, ParamData<P_COUNT> *old_param_data)
{
	int32_t *param_arr = crt_para_arr();

	thread	check_data_change([&]()
	{
		while (true)
		{
			this_thread::sleep_for(chrono::milliseconds(TICK_PERIOD));
			check_param_data(param_data, old_param_data);
		}
	});
	thread	send_test_data_thread([&]()
	{
		mtx_out.lock();
		cout << "Sender test data thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_sender = create_sender_socket(MULTICAST_TEST_IP, MULTICAST_TEST_PORT);
		send_test_data(udp_data_sender, shared_data, param_data);
		close(udp_data_sender.sock_fd);
	});
	thread sender_thread([&]()
	{
		mtx_out.lock();
		cout << "Sender thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_sender = create_sender_socket();
		send_ssv(udp_data_sender, shared_data, param_data);
		close(udp_data_sender.sock_fd);
		exit(0); // Exit after sending messages
	});
	thread receiver_thread([&]()
	{
		mtx_out.lock();
		cout << "Receiver thread started." << endl;
		mtx_out.unlock();
		udp_data_t udp_data_receiver = create_receive_socket();
		receiver_ssv(udp_data_receiver, shared_data, param_data);
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
	sender_thread.join();
	send_test_data_thread.join();
	check_data_change.join();
	delete[] param_arr;
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
