/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 21:36:46 by Pablo Escob      ###   ########.fr       */
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
	const uint8_t diff = static_cast<uint8_t>(i_primary - i_secondary);
	cout << "I_PRIMARY: " << (int)i_primary
			<< " I_SECONDARY: " << (int)i_secondary
			<< " RESULT: " << (diff > 1) << endl;
	return diff > 1;
}

int	main()
{
	cout << "SIZE: " << sizeof(P_Iterator) << endl;
	for (int i = 50; i > -50; --i)
	{
		check_iterator(INT8_MIN - i, INT8_MAX - i);
	}
	return 0;
}