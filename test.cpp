/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/10 07:39:50 by blackrider       ###   ########.fr       */
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
// 	if (i_primary < 0 && i_secondary > 0 && static_cast<uint16_t>(i_primary - i_secondary) > 1)
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

int	main()
{
	cout << "SIZE: " << sizeof(P_Iterator) << endl;
	for (int i = 50; i > -50; --i)
	{
		check_iterator(INT8_MIN - i, INT8_MAX - i + 3);
	}
	return 0;
}