/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_info.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 07:01:18 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/22 16:11:28 by blackrider       ###   ########.fr       */
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

#define INVALID_SOCKET	-1
#define MULTICAST_TEST_PORT  12347
#define MULTICAST_TEST_IP    "239.1.1.0"

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

most_popular_t find_most_popular_simple(const int* arr, int size)
{
    int most_popular = arr[0];
    int max_count = 0;
		most_popular_t result;
    
    for (int i = 0; i < size; i++) {
        int count = 0;
        for (int j = 0; j < size; j++) {
            if (arr[j] == arr[i]) {
                count++;
            }
        }
        if (count > max_count) {
            max_count = count;
            most_popular = arr[i];
        }
    }
		result.value = most_popular;
		result.count = max_count;
		return result;
}

int	*get_int_arr(test_data_t **test_info, int size,  uint16_t param_idx)
{
	int *arr = new int[size];
	mtx_common_data.lock();
	for (int i = 0; i < size; ++i)
		arr[i] = test_info[i][param_idx].param_val;
	mtx_common_data.unlock();
	return arr;
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
	
	cout << "-----------==============++++++++++ RECEIVED TEST INFO ++++++++++==============-----------" << endl;
	for (int i = 0; i < P_COUNT; ++i)
	{
		int *arr = get_int_arr(test_info, size, i);
		most_popular_t most_popular = find_most_popular_simple(arr, size);
		cout << "Most popular value for param " << test_info[0][i].param_num
				 << ": " << most_popular.value << ", count: " << most_popular.count << ", " << most_popular.count * percent << " [%]" << endl;
		delete[] arr;
	}
	cout << "-----------==============++++++++++ RECEIVED TEST INFO ++++++++++==============-----------" << endl;
}

void	receive_start_message(test_data_t	**test_info, const udp_data_t& udp_data)
{
	int size = get_test_data_size(test_info);
	test_data_t test_data {};
	socklen_t addr_len = sizeof(udp_data.remote_addr);

	thread receiver_thread([&]()
	{
		while (true)
		{
			handle_test_information(test_info, size);
			this_thread::sleep_for(chrono::seconds(1));
		}
	});

	while (true)
	{
		if (recvfrom(udp_data.sock_fd, &test_data, sizeof(test_data), 0,
				(struct sockaddr *)(&udp_data.remote_addr), &addr_len) < 0)
			die("recvfrom failed");
		mtx_common_data.lock();
		test_info[test_data.pid][test_data.param_idx] = test_data;
		mtx_common_data.unlock();
	}
	receiver_thread.join();
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
