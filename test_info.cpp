/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_info.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 07:01:18 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/25 15:53:26 by blackrider       ###   ########.fr       */
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
#include <unordered_map>

using namespace std;

#define INVALID_SOCKET				-1
#define MULTICAST_TEST_PORT 	12347
#define MULTICAST_TEST_IP    	"239.1.1.0"
#define MAX_NO_UPDATED 				10

mutex mtx_common_data;

struct most_popular_t
{
		int value;
		int count;
};

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

unordered_map<int, int> get_params_count(test_data_t **test_info, int size, uint16_t param_idx)
{
	unordered_map<int, int> count_map;

	for (int i = 0; i < size; ++i)
	{
		count_map[test_info[i][param_idx].param_val]++;
	}
	return count_map;
}

most_popular_t find_most_popular_simple(test_data_t **test_info, int size, uint16_t param_idx)
{
	most_popular_t result = {0, 0};
	unordered_map<int, int> count_map = get_params_count(test_info, size, param_idx);

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

int	get_test_data_size(test_data_t **test_info)
{
	int size = 0;
	
	while (test_info[size])
	{
		++size;
	}
	return size;
}

void	handle_test_information(test_data_t	**test_info, int size = 0)
{
	static float	percent = 100.0 / (float)size;
	static vector<most_popular_t> most_populars {P_COUNT};
	most_popular_t result;
	
	// cout << "-----------==============++++++++++ RECEIVED TEST INFO ++++++++++==============-----------" << endl;
	for (int i = 0; i < P_COUNT; ++i)
	{
		result = find_most_popular_simple(test_info, size, i);
		if (result.value != most_populars[i].value)
		{
			most_populars[i].value = result.value;
			most_populars[i].count = result.count;
			cout << "Most popular value for param " << test_info[0][i].param_num
					<< ": " << result.value << ", count: " << result.count << ", " << result.count * percent << " [%]" << endl;
		}
	}
	// cout << "-----------==============++++++++++ RECEIVED TEST INFO ++++++++++==============-----------" << endl;
}

void	check_data_update(vector<vector<int>>& is_updated, test_data_t **test_info)
{
	vector<uint32_t> average_update (is_updated.size(), 0);

	for (int i = 0; i < is_updated.size(); ++i)
	{
		for (auto& item : is_updated[i])
		{
			average_update[i] += item;
		}
		average_update[i] = average_update[i] / is_updated.size();
	}
	for (int i = 0; i < is_updated.size(); ++i)
	{
		for (int j = 0; j < is_updated[i].size(); ++j)
		{
			if (is_updated[i][j] < average_update[i] + MAX_NO_UPDATED)
			{
				test_info[i][j] = {0, 0, 0, 0, 0, 0}; // Reset the data if not updated enough
			}
		}
	}
}

void	reset_no_data_update(test_data_t **test_info, int size)
{
	vector<uint32_t> average_update_per_param(P_COUNT, 0);

	// Calculate average iterator value for each parameter across all processes
	for (int j = 0; j < P_COUNT; ++j)  // For each parameter
	{
		for (int i = 0; i < size; ++i)  // Sum across all processes
		{
			average_update_per_param[j] += test_info[i][j].i;
		}
		average_update_per_param[j] = average_update_per_param[j] / size;  // Average for this parameter
	}
	
	// Reset data that is too far behind the average for its parameter
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < P_COUNT; ++j)
		{
			if (test_info[i][j].i < (average_update_per_param[j] - MAX_NO_UPDATED))
			{
				test_info[i][j] = {0, 0, 0, 0, 0, 0}; // Reset the data if not updated enough
			}
		}
	}
}

void receive_data(test_data_t	**test_info, const udp_data_t& udp_data, int size)
{
	int	i = 0;
	test_data_t test_data {};
	vector<vector<int>> is_updated(size, vector<int>(P_COUNT, 0));
	socklen_t addr_len = sizeof(udp_data.remote_addr);
	
	while (true)
	{
		if (recvfrom(udp_data.sock_fd, &test_data, sizeof(test_data), 0,
				(struct sockaddr *)(&udp_data.remote_addr), &addr_len) < 0)
			die("recvfrom failed");
		mtx_common_data.lock();
		test_info[test_data.pid][test_data.param_idx] = test_data;
		mtx_common_data.unlock();
		++(is_updated[test_data.pid][test_data.param_idx]);
		if (i != 0 && !(i % 100))
			reset_no_data_update(test_info, size);
		++i;
	}
}

void	receive_start_message(test_data_t	**test_info, const udp_data_t& udp_data)
{
	int size = get_test_data_size(test_info);

	thread handler_thread([&]()
	{
		while (true)
		{
			handle_test_information(test_info, size);
			this_thread::sleep_for(chrono::seconds(1));
		}
	});
	thread data_receiver_thread([&]()
	{
		receive_data(test_info, udp_data, size);
	});
	handler_thread.join();
	data_receiver_thread.join();
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
	receive_start_message(test_info, udp_data_receiver);
	close(udp_data_receiver.sock_fd);
	for (int i = 0; i < count; ++i)
	{
		delete[] test_info[i];
	}
	delete[] test_info;
	return 0;
}
