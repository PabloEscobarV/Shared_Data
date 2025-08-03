/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_test.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 13:55:51 by blackrider        #+#    #+#             */
/*   Updated: 2025/08/04 00:27:52 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/test.hpp"

#include <iostream>
#include <cstdint>
#include <limits>
#include <unistd.h>
#include <wait.h>
#include <vector>

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

int	create_proc(int id, input_data_t& in_data)
{
	int pid = fork();
	
	if (pid < 0)
	{
		cerr << "Fork failed" << endl;
		return -1;
	}
	if (pid == 0) // Child process
	{
		run_app(id, in_data.start_iter_val, in_data.kef);
		exit(0); // Exit child process after running the app
	}
	else // Parent process
	{
		return pid; // Return the PID of the child process
	}
}

int	menu_f(vector<int>& pids, input_data_t& in_data)
{
	int id = 0;
	int menu = 0;

	cout << "Enter 0 for exist, 1 for add new process, or 2 for end process: ";
	cin >> menu;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	switch (menu)
	{
	case 0:
		cout << "Exiting..." << endl;
		break;;
	case 1:
		cout << "Enter the ID of the new process: ";
		cin >> id;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Adding new process with ID: " << id << endl;
		pids.push_back(create_proc(id, in_data));
		break;
	case 2:
		cout << "Enter the ID of the process to end: ";
		cin >> id;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Ending process with ID: " << id << endl;
		kill(pids[id], SIGTERM);
		pids.erase(pids.begin() + id);
		break;
	default:
		cout << "Invalid option. Please try again." << endl;
		break;
	}
	return menu;
}

void	end_apps(vector<int>& pids, input_data_t& in_data)
{
	int menu = 0;

	while (menu_f(pids, in_data) != 0);
	for (int pid : pids)
	{
		if (pid > 0)
		{
			kill(pid, SIGTERM);
			cout << "Process with PID " << pid << " has been terminated." << endl;
		}
	}
}

void	create_proccesses(input_data_t& in_data)
{
	vector<int> pids(in_data.count);

	for (uint16_t i = 0; i < in_data.count; ++i)
	{
		pids[i] = create_proc(i, in_data);
	}
	end_apps(pids, in_data);
	for (uint16_t i = 0; i < in_data.count; ++i)
	{
		wait(NULL); // Wait for all child processes to finish
	}
}

int main()
{
	int	count = 0;
	string command;
	input_data_t data = get_input_data();
	create_proccesses(data);

	// system("gnome-terminal -- bash -c 'g++ test.cpp src/* -o test'");
	// system("gnome-terminal -- bash -c 'g++ test_ssrv.cpp src/* -o test_ssrv'");
	// system("gnome-terminal -- bash -c 'g++ test_info.cpp src/* -o test_info'");
	// crt_test_proccess(get_input_data());
	// create_command(data);
	return 0;
}
