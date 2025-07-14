/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue_test.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 06:54:30 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/14 07:15:05 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/queue.hpp"
#include <iostream>

using namespace std;

int	main()
{
	FSQueue<int, 5> queue;
	int item;

	// Test pushing items
	for (int i = 0; i < 5; ++i)
	{
		queue.push(i);
		cout << "Pushed: " << i << endl;
	}

	// Test popping items
	while (queue.pop(item))
	{
		cout << "Popped: " << item << endl;
	}

	// Test pushing again after popping
	for (int i = 0; i < 5; ++i)
	{
		queue.push(i);
		cout << "Pushed: " << i << endl;
	}

	for (int i = 0; i < 10; ++i)
	{
		queue.swap(i);
		cout << "Swapped item at index " << i << endl;
		queue.peek(item);
		cout << "Current head item: " << item << endl;
	}

	// Test popping again
	while (queue.pop(item))
	{
		cout << "Popped: " << item << endl;
	}

	return 0;
}