/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/10 10:27:56 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hdrs/p_iterator.hpp"

#include <iostream>

using namespace std;

// bool	check_iterator(const int8_t i_primary, const int8_t i_secondary)
// {
// 	bool result = false;

// 	if (i_primary > i_secondary + 1)
// 	{
// 		result = true;
// 	}
// 	if (i_primary < 0 && i_secondary > 0 && static_cast<uint32_t>(i_primary - i_secondary) > 1)
// 	{
// 		result = true;
// 	}
// 	cout << "I_PRIMARY: " << (int)i_primary
// 			<< " I_SECONDARY: " << (int)i_secondary
// 			<< " RESULT: " << result << endl;
// 	return result;
// }

bool check_iterator(const int8_t i_primary, const int8_t i_secondary)
{
	const int16_t diff = abs(i_primary) - abs(i_secondary);
	cout << "I_PRIMARY: " << (int)i_primary
			<< " I_SECONDARY: " << (int)i_secondary
			<< " RESULT: " << diff << endl;
	return diff > 1;
}

uint32_t	get_idx(uint32_t p_num, uint32_t *shared_params, uint32_t count, uint32_t& step)
{
	uint32_t	left = 0;
	uint32_t	right = count;
	uint32_t 	mid = 0;
	uint32_t	current_param_num = 0;
	
	step = 0;
	while (left < right)
	{
		mid = left + (right - left) / 2;
		current_param_num = shared_params[mid];
		
		if (current_param_num == p_num)
		{
			break ;
		}
		if (current_param_num < p_num)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
		++step;
	}
	if (current_param_num != p_num)
	{
		mid = count;
	}
	return mid;
}

int	main()
{
	// cout << "SIZE: " << sizeof(P_Iterator) << endl;
	// for (int i = 50; i > -50; --i)
	// {
	// 	check_iterator(INT8_MIN - i, INT8_MAX - i + 3);
	// }
	uint32_t count = 100;
	uint32_t *shared_params = new uint32_t[count];
	uint32_t step = 0;
	uint32_t min_step = 0;
	uint32_t max_step = 0;

	for (uint32_t i = 0; i < count; ++i)
	{
		shared_params[i] = i * 2; // Initialize with even numbers for testing
	}
	// cout << "SHARED PARAMS: " << endl;
	// for (uint32_t i = 0; i < count; ++i)
	// {
	// 	cout << "PARAM_NUM: " << i << " VALUE: " << shared_params[i] << endl;
	// }
	cout << "SEARCHING FOR PARAM_NUMS: " << endl;
	for (int i = 0; i < count * 3; ++i)
	{
		uint32_t idx = get_idx(i, shared_params, count, step);
		if (step < min_step || i == 0)
		{
			min_step = step;
		}
		if (step > max_step)
		{
			max_step = step;
		}
		if (idx < count)
		{
			cout << "PARAM_NUM: " << i << " INDEX: " << idx << endl;
		}
		else
		{
			cout << "PARAM_NUM: " << i << " NOT FOUND" << endl;
		}
	}
	cout << "MIN STEP: " << min_step << endl;
	cout << "MAX STEP: " << max_step << endl;
	delete[] shared_params;
	cout << "END OF TEST" << endl;
	return 0;
}