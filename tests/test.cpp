/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/23 15:32:14 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

#include <cstdint>

#include <iostream>

using namespace std;

static const uint16_t 	LARGE_ITER_BIT = INT16_MAX + 1;
static const uint16_t	ITER_DIFF = 1;

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

int16_t get_diff(const uint16_t i_primary, const uint16_t i_secondary)
{
	int32_t diff = static_cast<int32_t>(i_primary - i_secondary);
	
	if (diff >= LARGE_ITER_BIT / 2)
	{
		diff -= LARGE_ITER_BIT;
	}
	if (diff <= -LARGE_ITER_BIT / 2)
	{
		diff += LARGE_ITER_BIT;
	}
	return diff;
}

bool	check_iterators(uint16_t i_primary, uint16_t i_secondary)
{
	bool result = false;

	// Check large bit status
	bool primary_large = (i_primary >= LARGE_ITER_BIT);
	bool secondary_large = (i_secondary >= LARGE_ITER_BIT);

	if (primary_large && !secondary_large)
	{
		// Primary has large bit, secondary doesn't - primary is newer
		result = true;
	}
	if ((!primary_large && !secondary_large) || (primary_large && secondary_large))
	{
		// Both have same large bit status, compare normally
		result = get_diff(i_primary, i_secondary) > ITER_DIFF;
	}
	return result;
}

int16_t check_iterator(const uint16_t i_primary, const uint16_t i_secondary)
{
	int32_t diff = get_diff(i_primary, i_secondary);
	cout << "I_PRIMARY: " << (int)i_primary
			<< " I_SECONDARY: " << (int)i_secondary
			<< " DIFF: " << (int)get_diff(i_primary, i_secondary)
			<< " RESULT(get_diff()): " << (diff > 1)
			<< " RESULT (check_iterators()): " << (check_iterators(i_primary, i_secondary) ? "true" : "false") << endl;
	return diff > 2;
}

void incr_iter(uint16_t& iterator, uint16_t incr_val)
{
	uint16_t	tmp = iterator & (LARGE_ITER_BIT - 1);
	
	if (iterator >= LARGE_ITER_BIT || incr_val >= LARGE_ITER_BIT)
	{
		iterator = (tmp + incr_val) % LARGE_ITER_BIT | LARGE_ITER_BIT;
	}
	else
	{
		iterator += incr_val;
	}
}

int main()
{
	uint16_t iterator1 = INT16_MAX * 2 - 200;
	uint16_t iterator2 = INT16_MAX * 2 - 195;

	for (int i = 0; i < 400; ++i)
	{
		incr_iter(iterator1, 1);
		incr_iter(iterator2, 1);
		check_iterator(iterator1, iterator2);
	}

	cout << "----------------===================++++++++++===================----------------\n";
	iterator1 = INT16_MAX * 2 - 50;
	iterator2 = 0;
	for (int i = 0; i < 100; ++i)
	{
		incr_iter(iterator1, 1);
		incr_iter(iterator2, 1);
		check_iterator(iterator1, iterator2);
	}

	// uint8_t iterator = 127;
	
	// for (int16_t i = 0; i < 400; ++i)
	// {
	// 	// check_iterator((i + 3) % 128, (i) % 128);
	// 	incr_iter(iterator, 1);
	// 	cout << "ITERATOR: " << (int)iterator << endl;
	// }
	return 0;
}

// uint32_t	get_idx(uint32_t p_num, uint32_t *shared_params, uint32_t count, uint32_t& step)
// {
// 	uint32_t	left = 0;
// 	uint32_t	right = count;
// 	uint32_t 	mid = 0;
// 	uint32_t	current_param_num = 0;
	
// 	step = 0;
// 	while (left < right)
// 	{
// 		mid = left + (right - left) / 2;
// 		current_param_num = shared_params[mid];
		
// 		if (current_param_num == p_num)
// 		{
// 			break ;
// 		}
// 		if (current_param_num < p_num)
// 		{
// 			left = mid + 1;
// 		}
// 		else
// 		{
// 			right = mid;
// 		}
// 		++step;
// 	}
// 	if (current_param_num != p_num)
// 	{
// 		mid = count;
// 	}
// 	return mid;
// }

// int	main()
// {
// 	// cout << "SIZE: " << sizeof(P_Iterator) << endl;
// 	// for (int i = 50; i > -50; --i)
// 	// {
// 	// 	check_iterator(INT8_MIN - i, INT8_MAX - i + 3);
// 	// }
// 	// uint32_t count = 100;
// 	// uint32_t *shared_params = new uint32_t[count];
// 	// uint32_t step = 0;
// 	// uint32_t min_step = 0;
// 	// uint32_t max_step = 0;

// 	// for (uint32_t i = 0; i < count; ++i)
// 	// {
// 	// 	shared_params[i] = i * 2; // Initialize with even numbers for testing
// 	// }
// 	// // cout << "SHARED PARAMS: " << endl;
// 	// // for (uint32_t i = 0; i < count; ++i)
// 	// // {
// 	// // 	cout << "PARAM_NUM: " << i << " VALUE: " << shared_params[i] << endl;
// 	// // }
// 	// cout << "SEARCHING FOR PARAM_NUMS: " << endl;
// 	// for (int i = 0; i < count * 3; ++i)
// 	// {
// 	// 	uint32_t idx = get_idx(i, shared_params, count, step);
// 	// 	if (step < min_step || i == 0)
// 	// 	{
// 	// 		min_step = step;
// 	// 	}
// 	// 	if (step > max_step)
// 	// 	{
// 	// 		max_step = step;
// 	// 	}
// 	// 	if (idx < count)
// 	// 	{
// 	// 		cout << "PARAM_NUM: " << i << " INDEX: " << idx << endl;
// 	// 	}
// 	// 	else
// 	// 	{
// 	// 		cout << "PARAM_NUM: " << i << " NOT FOUND" << endl;
// 	// 	}
// 	// }
// 	// cout << "MIN STEP: " << min_step << endl;
// 	// cout << "MAX STEP: " << max_step << endl;
// 	// delete[] shared_params;
// 	// cout << "END OF TEST" << endl;
// 	uint8_t tick = 0;
// 	uint8_t ssrv_end_counter = 255;
// 	cout << "DIFF:\t" << (int)(static_cast<uint8_t>(tick - ssrv_end_counter)) << endl;
// 	return 0;
// }