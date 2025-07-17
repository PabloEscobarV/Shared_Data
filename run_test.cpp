/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_test.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 13:55:51 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/17 12:51:34 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <time.h>
#include <unistd.h>

using namespace std;

void	start_test(int count)
{
	string	part_1 {"gnome-terminal -- bash -c './test "};
	string	part_2 {"; echo; echo Application Finished; echo Press Enter to continue ...; read'"};
	string	full_command {};

	for (int i = 0; i < count; ++i)
	{
		part_1 = "gnome-terminal -- bash -c './test ";
		part_1 += to_string(i * 2 - 100);
		part_1 += " ";
		part_1 += to_string(1);
		part_1 += " ";
		part_1 += to_string(rand() % 100 + 25);
		full_command = part_1 + part_2;
		system(full_command.c_str());
	}
}

int main()
{
	int	count = 0;
	srand(time(NULL) + getpid()); // Seed random number generator with current time

	cout << "Enter the number of simulations: ";
	cin >> count;
	system("gnome-terminal -- bash -c 'g++ test.cpp src/*.cpp -o test'");
	start_test(count);
	// while (count)
	// {
	// 	system("gnome-terminal -- bash -c './test; echo; echo Application Finished; echo Press Enter to continue ...; read'");
	// 	--count;
	// }
	return 0;
}