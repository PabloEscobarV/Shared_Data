/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 10:28:21 by blackrider        #+#    #+#             */
/*   Updated: 2025/10/26 13:13:35 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_HPP
#define TEST_HPP

#include <cstdint>
#include <sys/socket.h>    // Basic socket functions
#include <netinet/in.h>    // sockaddr_in structure
#include <arpa/inet.h>     // inet_addr(), inet_ntoa(), etc.

#define P_COUNT 64
#define NUM_SYNC_PARAM 64

using namespace std;

struct	udp_data_t
{
	int 				sock_fd;
	sockaddr_in remote_addr;
};

struct	ssrv_data_t
{
	uint16_t	id;
	uint16_t	param_idx;
	int32_t		param_val;
};

struct test_data_t
{
	uint16_t	pid;
	uint16_t 	param_num;
	uint16_t	param_idx;
	uint16_t	iterator;
	uint16_t	i;
	int32_t		param_val;
};

struct heartbeat_t
{
	uint16_t	pid;  // Process ID (0 to max-1)
	uint32_t	i;    // Heartbeat counter
};

struct signal_state_t
{
	bool is_msg_requested;
};

uint16_t sync_param_list[NUM_SYNC_PARAM];

uint16_t	get_pid();
void	run_app(uint16_t pid, uint16_t iterator_start_value, uint16_t param_kef);
// void	start_test(uint16_t pid, uint16_t iterator_start_value, uint16_t param_kef);

#endif // TEST_HPP