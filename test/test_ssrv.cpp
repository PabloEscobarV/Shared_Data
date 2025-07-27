/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_ssrv.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 06:51:11 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/28 00:17:16 by Pablo Escob      ###   ########.fr       */
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

using namespace std;

#define INVALID_SOCKET	-1
#define MULTICAST_SSRV_PORT  12346
#define MULTICAST_SSRV_IP    "239.1.1.1"

ssrv_data_t	get_ssrv_data()
{
	ssrv_data_t ssrv_start_message = {};
	
	cout  << "Enter SSRV start message parameters:" << endl;
	cout << "ID: ";
	cin >> ssrv_start_message.id;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Parameter Index: ";
	cin >> ssrv_start_message.param_idx;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Parameter Value: ";
	cin >> ssrv_start_message.param_val;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return ssrv_start_message;
}

void	send_ssrv_start_message(const udp_data_t& udp_data)
{
	ssrv_data_t ssrv_start_message = {};

	while (true)
	{
		ssrv_start_message = get_ssrv_data();
		cout << "Entered ID: " << ssrv_start_message.id
			 << ", Parameter Index: " << ssrv_start_message.param_idx
			 << ", Parameter Value: " << ssrv_start_message.param_val << endl;
		send_udp(udp_data, ssrv_start_message);
	}
}

int	main()
{
	udp_data_t udp_data_sender = create_sender_socket(MULTICAST_SSRV_IP, MULTICAST_SSRV_PORT);
	send_ssrv_start_message(udp_data_sender);
	close(udp_data_sender.sock_fd);
	return 0;
}
