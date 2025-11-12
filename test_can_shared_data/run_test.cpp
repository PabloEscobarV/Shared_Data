/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_test.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 13:55:51 by blackrider        #+#    #+#             */
/*   Updated: 2025/11/12 08:58:57 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/test.hpp"

#include "info_all_cu.hpp"
#include "test_ssrv.hpp"

#include <iostream>
#include <cstdint>
#include <limits>
#include <unistd.h>
#include <wait.h>
#include <vector>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <thread>
#include <fstream>
#include <sys/stat.h>

using namespace std;

struct input_data_t
{
	int		 		id;
	uint16_t	count;
	uint16_t	start_iter_val;
	uint16_t	kef;
};

sem_t* create_semaphore()
{
	sem_unlink(SEM_NAME); // удалить существующий семафор с таким именем, если есть
	sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0644, 1); // создать семафор с начальным значением 1
	if (sem == SEM_FAILED) 
	{
		cerr << "Failed to create semaphore" << endl;
		exit(1);
	}
	return sem;
}

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
		run_app(id, in_data.kef);
		exit(0); // Exit child process after running the app
	}
	else // Parent process
	{
		return pid; // Return the PID of the child process
	}
}

int	menu_f(vector<int>& pids, input_data_t& in_data, sem_t *sem_output)
{
	int id = 0;
	int menu = 0;

	// sem_wait(sem_output);
	cout << "Enter 0 for exist, 1 for add new process, 2 for end process, 3 for send signal or 4 for send SSRV reqest:\n";
	cin >> menu;
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	switch (menu)
	{
	case 0:
		cout << "Exiting..." << endl;
		break;
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
	case 3:
		cout << "Enter the ID of the process to send signal: ";
		cin >> id;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		kill(pids[id], SIGUSR1);
		break;
	case 4:
		send_ssrv_request();
		break;
	default:
		cout << "Invalid option. Please try again." << endl;
		break;
	}
	return menu;
}

void	end_apps(int stat_pid, vector<int>& pids, input_data_t& in_data, sem_t *sem_output)
{
	int menu = 0;

	while (menu_f(pids, in_data, sem_output) != 0);
	for (int pid : pids)
	{
		if (pid > 0)
		{
			kill(pid, SIGTERM);
			cout << "Process with PID " << pid << " has been terminated." << endl;
		}
	}
	kill(stat_pid, SIGTERM);
}

void run_stat_process()
{
	const char* fifo= "info_all_cu_fifo";
	unlink(fifo); 
	if (mkfifo(fifo, 0666) == -1) 
	{
		cerr << "Failed to create FIFO" << endl;
		return;
	}
	system("gnome-terminal -- bash -c 'echo Reading FIFO...; cat info_all_cu_fifo;' &");
	ofstream out(fifo);
	if (!out) { std::cerr << "open fifo failed\n"; return; }
	streambuf* coutbuf = std::cout.rdbuf(out.rdbuf());
	receive_all_cu_data();
	cout.rdbuf(coutbuf);
}

int create_statistik_process()
{
	int pid = fork();

	if (pid < 0)
	{
		cerr << "Fork failed" << endl;
		return -1;
	}
	if (pid == 0) // Child process
	{
		run_stat_process();
		return 0;
	}
	return pid; // Return the PID of the child process
}


void	create_proccesses(input_data_t& in_data, sem_t *sem_output)
{
	vector<int> pids(in_data.count);
	int stat_pid = create_statistik_process();
	
	for (uint16_t i = 0; i < in_data.count; ++i)
	{
		pids[i] = create_proc(i, in_data);
	}
	end_apps(stat_pid, pids, in_data, sem_output);
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
	sem_t* sem_output = create_semaphore();
	create_proccesses(data, sem_output);

	// system("gnome-terminal -- bash -c 'g++ test.cpp src/* -o test'");
	// system("gnome-terminal -- bash -c 'g++ test_ssrv.cpp src/* -o test_ssrv'");
	// system("gnome-terminal -- bash -c 'g++ test_info.cpp src/* -o test_info'");
	// crt_test_proccess(get_input_data());
	// create_command(data);
	sem_unlink(SEM_NAME); // удалить семафор из системы
	return 0;
}
