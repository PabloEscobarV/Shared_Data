/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_test.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 13:55:51 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/28 00:10:35 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/test.hpp"

#include <iostream>
#include <cstdint>
#include <limits>
#include <unistd.h>
#include <wait.h>

using namespace std;

struct input_data_t
{
	int		 		id;
	uint16_t	count;
	uint16_t	start_iter_val;
	uint16_t	kef;
};

input_data_t get_input_data()
{
	input_data_t ssrv_start_message;

	cout << "Enter SSRV start message parameters:" << endl;
	cout << "ID: ";
	cin >> ssrv_start_message.id;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Count: ";
	cin >> ssrv_start_message.count;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Parameter start Iterator: ";
	cin >> ssrv_start_message.start_iter_val;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cout << "Kef: ";
	cin >> ssrv_start_message.kef;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	return ssrv_start_message;
}

void	create_command(input_data_t& data)
{
	string	part1 = "gnome-terminal -- bash -c './test";
	string 	part2 = "; echo; echo Application Finished; echo Press Enter to continue ...; read'";
	for (int i = 0; i < data.count; ++i)
	{
		part1 = "gnome-terminal -- bash -c './test";
		part1 += " " + to_string(data.id + i) + " " + to_string(data.start_iter_val);
		if (data.kef > 1)
		{
			data.kef += i;
		}
		part1 += " " + to_string(data.kef);
		part1 += part2;
		cout << "Executing command: " << part1 << endl;
		system(part1.c_str());
	}
}

int main()
{
	int	count = 0;
	string command;
	input_data_t data = get_input_data();

	system("gnome-terminal -- bash -c 'g++ test.cpp src/* -o test'");
	// system("gnome-terminal -- bash -c 'g++ test_ssrv.cpp src/* -o test_ssrv'");
	// system("gnome-terminal -- bash -c 'g++ test_info.cpp src/* -o test_info'");
	// crt_test_proccess(get_input_data());
	create_command(data);
	return 0;
}
