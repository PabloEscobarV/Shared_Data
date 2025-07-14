/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_test.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 13:55:51 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/14 14:05:27 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

using namespace std;

int main()
{
	int	count = 0;

	cout << "Enter the number of simulations: ";
	cin >> count;
	system("gnome-terminal -- bash -c 'g++ test.cpp src/* -o test'");
	while (count)
	{
		system("gnome-terminal -- bash -c './test; echo; echo Application Finished; echo Press Enter to continue ...; read'");
		--count;
	}
	return 0;
}