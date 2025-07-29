/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_info.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 07:01:18 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/30 00:36:23 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/shared_data.hpp"
#include "../hdrs/client_server_shared_setpoint.hpp"
#include "../hdrs/test.hpp"
#include "../hdrs/socket.hpp"

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
#include <unordered_map>

using namespace std;

#define INVALID_SOCKET				-1
#define MULTICAST_TEST_PORT 	12347
#define MULTICAST_TEST_IP    	"239.1.1.0"
#define MAX_NO_UPDATED 				10

ParamData<P_COUNT>	*param_data;

struct most_popular_t
{
	int value;
	int count;
};

unordered_map<int, int> get_params_count(int size, uint16_t idx)
{
	unordered_map<int, int> count_map;

	for (int i = 0; i < size; ++i)
	{
		count_map[param_data[i].get_param_value(idx)]++;
	}
	return count_map;
}

most_popular_t find_most_popular_simple(int size, uint16_t idx)
{
	most_popular_t result = {0, 0};
	unordered_map<int, int> count_map = get_params_count(size, idx);

	for (const auto& pair : count_map)
	{
		if (pair.second > result.count)
		{
			result.value = pair.first;
			result.count = pair.second;
		}
	}	
	return result;
}

bool	handle_in_data(test_data_t& test_info)
{
	bool	changed = false;
	if (test_info.param_val != param_data[test_info.pid].get_param_value(test_info.param_idx))
	{
		param_data[test_info.pid].set_param_value(test_info.param_idx, test_info.param_num, test_info.param_val);
		changed = true;
	}
	return changed;
}

void	check_data_change(most_popular_t *stat_data, test_data_t **test_info, int size)
{
	most_popular_t most_popular;
	static float percent = 100.0 / size;

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < P_COUNT; ++j)
		{
			handle_in_data(test_info[i][j]);
		}
	}
	for (int i = 0; i < size; ++i)
	{
		cout << "PARAM INDEX " << i << ": "
				 << "PARAM VALUE: " << param_data[0].get_param_value(i)
				 << ", PARAM NUM: " << param_data[0].get_param_num(i) << endl;
	}
	for (int i = 0; i < P_COUNT; ++i)
	{
		most_popular = find_most_popular_simple(size, i);
		cout << "Param " << i << ": "
				 << "Value: " << most_popular.value
				 << ", Count: " << most_popular.count
				 << " Percent: " << (float)(most_popular.count * percent) << "%" << endl;
		if (most_popular.count != stat_data[i].count ||
				most_popular.value != stat_data[i].value)
		{
			stat_data[i].value = most_popular.value;
			stat_data[i].count = most_popular.count;
			cout << "Param " << i << " changed: "
				 << "Value: " << most_popular.value
				 << ", Count: " << most_popular.count
				 << " Percent: " << (float)(most_popular.count * percent) << "%"
				 << endl;
		}
	}
}

bool	check_all_received(bool	**received, int size)
{
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < P_COUNT; ++j)
		{
			// If any PID has not received data for a parameter, return false
			if (!received[i][j])
				return false;
		}
	}
	return true;
}

bool **crt_received(int size)
{
	bool **received = new bool*[size];
	for (int i = 0; i < size; ++i)
	{
		received[i] = new bool[P_COUNT];
		for (int j = 0; j < P_COUNT; ++j)
		{
			received[i][j] = false;
		}
	}
	return received;
}

void	get_test_data(test_data_t **test_info, udp_data_t &udp_data_receiver, int size)
{
	bool	**received = crt_received(size);
	test_data_t test_data;

	while (true)
	{
		receive_udp(udp_data_receiver, test_data);
		received[test_data.pid][test_data.param_idx] = true;
		test_info[test_data.pid][test_data.param_idx] = test_data;
		cout << "TEST:\nReceived data from PID: " << test_info[test_data.pid][test_data.param_idx].pid
				 << ", Param Index: " << test_info[test_data.pid][test_data.param_idx].param_idx
				 << ", Param Number: " << test_info[test_data.pid][test_data.param_idx].param_num
				 << ", Param Value: " << test_info[test_data.pid][test_data.param_idx].param_val
				 << ", Iterator: " << test_info[test_data.pid][test_data.param_idx].iterator << endl;
		if (check_all_received(received, size))
			break;
	}
	for (int i = 0; i < size; ++i)
	{
		delete[] received[i];
	}
	delete[] received;
}

void	handle_data(udp_data_t &udp_data_receiver, int size)
{
	most_popular_t *stat_data = new most_popular_t[P_COUNT];
	test_data_t **test_data = new test_data_t*[size];
	for (int i = 0; i < P_COUNT; ++i)
	{
		test_data[i] = new test_data_t[P_COUNT];
		stat_data[i] = {0, 0};
	}
	while (true)
	{
		get_test_data(test_data, udp_data_receiver, size);
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < P_COUNT; ++j)
			{
				cout << "Received data from PID: " << test_data[i][j].pid
					<< ", Param Index: " << test_data[i][j].param_idx
					<< ", Param Number: " << test_data[i][j].param_num
					<< ", Param Value: " << test_data[i][j].param_val
					<< ", Iterator: " << test_data[i][j].iterator << endl;
			}
		}
		check_data_change(stat_data, test_data, size);
		this_thread::sleep_for(chrono::seconds(5));
	}
	delete[] stat_data;
	delete[] test_data;
}

int main()
{
	uint16_t	count;
	udp_data_t udp_data_receiver = create_receive_socket(MULTICAST_TEST_IP, MULTICAST_TEST_PORT);

	cout << "Enter the number of proccess to receive: ";
	cin >> count;
	param_data = new ParamData<P_COUNT>[count];
	handle_data(udp_data_receiver, count);
	close(udp_data_receiver.sock_fd);
	return 0;
}
