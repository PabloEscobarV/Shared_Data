/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/16 07:30:04 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>

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

int8_t get_diff(const uint8_t i_primary, const uint8_t i_secondary)
{
	return static_cast<uint8_t>(i_primary - i_secondary);
}

int8_t check_iterator(const int8_t i_primary, const int8_t i_secondary)
{
	int32_t diff = get_diff(i_primary, i_secondary);
	cout << "I_PRIMARY: " << (int)i_primary
			<< " I_SECONDARY: " << (int)i_secondary
			<< " DIFF: " << (int)get_diff(i_primary, i_secondary)
			<< " RESULT: " << (diff > 1) << endl;
	return diff > 2;
}

int main()
{
	int16_t iterator = 0; // Example iterator value
	for (int i = 0; i < 255; ++i)
	{
		// cout << "ITERATOR: " << iterator << " ITERATOR8_t: " << (int)static_cast<int8_t>(iterator) << "[" << i << "]" << endl;
		check_iterator(static_cast<uint8_t>((iterator + 2)), static_cast<uint8_t>((iterator)));
		iterator = (iterator & 0xFF00) | (static_cast<uint8_t>(iterator) + 1);
	}
	// for (int16_t i = -32767; i < -31800; ++i)
	// {
	// 	check_iterator(i, i - 2);
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