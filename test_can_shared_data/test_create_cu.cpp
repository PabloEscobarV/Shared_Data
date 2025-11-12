/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_create_cu.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 14:27:52 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/11/12 09:55:56 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../can_shared_data/can_shared_data.hpp"
#include "../param_data/param_data.hpp"
#include "../hdrs/socket.hpp"
#include "../hdrs/test.hpp"

#include <thread>
#include <chrono>
#include <sys/eventfd.h>
#include <unistd.h>
#include <mutex>
#include <random>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

using namespace std;

enum events
{
	EVENT_NONE = 0,
	EVENT_NEW_MESSAGE = 1 << 0,
};

mutex			print_mutex;
ParamData	*param_data;
uint16_t	sync_param_list[NUM_SYNC_PARAM];


static const uint16_t TICK_PERIOD = 20;

void print_param_data()
{
	for (uint16_t i = 0; i < NUM_SYNC_PARAM; ++i)
	{
		cout << i << " "
			 << param_data->get_param_num(i) << " "
			 << param_data->get_param_value(i) << " "
			 << endl;
	}
}

void print_signal_handler(sigset_t& set, sem_t* print_sem)
{
	while (true)
	{
		siginfo_t si{};
		int sig = sigwaitinfo(&set, &si);
		if (sig == SIGUSR1) 
		{
			if (sem_wait(print_sem) == -1) {
				perror("sem_wait failed");
				continue;
			}
			print_param_data();
			if (sem_post(print_sem) == -1) {
				perror("sem_post failed");
			}
		}
		else if (sig == -1) {
			perror("sigwaitinfo failed");
		}
	}
}

void send_data(int event_fd, Can_shared_data& can_shared_data, ParamData *param_data, uint16_t pid)
{
	Can_app_message can_app_message;
	uint64_t event = 0;
	udp_data_t udp_data = create_sender_socket();

	while (true)
	{
		read(event_fd, &event, sizeof(event));
		if (event & EVENT_NEW_MESSAGE)
		{
			if (can_shared_data.get_message(can_app_message))
			{
				can_app_message.set_pid(static_cast<uint8_t>(pid));
				send_udp(udp_data, can_app_message);
			}
		}
		can_app_message = Can_app_message();
	}
}

void service_20ms(Can_shared_data& can_shared_data)
{
	while (true)
	{
		can_shared_data.service();
		this_thread::sleep_for(chrono::milliseconds(20));
	}
}

void receive_data(Can_shared_data& can_shared_data, uint16_t pid)
{
	Can_app_message can_app_message;
	udp_data_t udp_data = create_receive_socket();

	while (true)
	{
		receive_udp(udp_data, can_app_message);
		if (can_app_message.get_source_controller_id() == pid)
			continue;
		can_shared_data.process_message(can_app_message);
		can_app_message = Can_app_message();
	}
}

void receive_ssrv_request(Can_shared_data& can_shared_data, uint16_t pid)
{
	ssrv_data_t ssrv_req_message {};
	udp_data_t udp_data = create_receive_socket(MULTICAST_SSRV_IP, MULTICAST_SSRV_PORT);

	while (true)
	{
		receive_udp(udp_data, ssrv_req_message);
		if (ssrv_req_message.id == pid)
		{
			cout << "Received SSRV request for: " << ssrv_req_message.id
					 << ", Param Index: " << ssrv_req_message.param_idx
					 << ", Param Value: " << ssrv_req_message.param_val << endl;
			can_shared_data.add_ssrv_message(param_data->get_param_num(ssrv_req_message.param_idx),
											reinterpret_cast<const uint8_t*>(&ssrv_req_message.param_val));
		}
	}
}

void crt_threads(int event_fd,
								Can_shared_data& can_shared_data,
								ParamData *param_data,
								uint16_t pid,
								sigset_t& set,
								sem_t* print_sem)
{
	thread	service_th([&]()
	{
		service_20ms(can_shared_data);
	});
	thread	sender_th([&]()
	{
		send_data(event_fd, can_shared_data, param_data, pid);
	});
	thread	receiver_th([&]()
	{
		receive_data(can_shared_data, pid);
	});
	thread	ssrv_th([&]()
	{
		receive_ssrv_request(can_shared_data, pid);
	});
	thread	signal_handler_th([&]()
	{
		print_signal_handler(set, print_sem);
	});
	sender_th.join();
	service_th.join();
	receiver_th.join();
	ssrv_th.join();
	signal_handler_th.join();
}

void	init_param_data(ParamData *param_data, uint16_t step_kef, uint16_t pid)
{
	int32_t param_value = 0;

	for (uint16_t i = 0; i < NUM_SYNC_PARAM; ++i)
	{
		param_value = static_cast<int32_t>(std::rand() % (9999 + pid) + 2077 + pid);
		sync_param_list[i] = i * step_kef;
		param_data->set_param_data(i, sync_param_list[i], param_value, 100000, 0);
	}
}

void run_app(uint16_t pid, uint16_t param_kef)
{
	Can_shared_data	can_shared_data;
	param_data = new ParamData();
	int event_fd = eventfd(0, 0);
	sigset_t set;
	sem_t* print_sem = sem_open(SEM_NAME, 0);
	
	if (print_sem == SEM_FAILED) {
		perror("sem_open failed in run_app");
		delete param_data;
		return;
	}
	
	// cerr << "[DEBUG] Process " << getpid() << " with logical ID " << pid << " started\n";

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &set, nullptr);
	srand(time(nullptr));
	init_param_data(param_data, param_kef, pid);
	can_shared_data.initialize(pid, event_fd);
	crt_threads(event_fd, can_shared_data, param_data, pid, set, print_sem);

	delete param_data; 
}

// int main()
// {
// 	uint16_t iterator_start_value;
// 	uint16_t param_kef = 0;
// 	uint16_t pid = 0;

// 	cout << "Enter process ID (PID): \n";
// 	cin >> pid;
// 	cout << "Enter parameter step kef: \n";
// 	cin >> param_kef;
// 	run_app(pid, param_kef);
// 	return 0;
// }
