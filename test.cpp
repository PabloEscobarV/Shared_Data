/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:26:38 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 15:35:26 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hdrs/p_iterator.hpp"

#include <iostream>

using namespace std;

bool	check_iterator(const int8_t i_primary, const int8_t i_secondary)
{
	bool result = false;
	uint16_t	diff = static_cast<uint16_t>(i_primary) - static_cast<uint16_t>(i_secondary);

	if (diff)
	{
		result = true;
	}
	cout << "I_PRIMARY: " << (int)i_primary
				<< " I_SECONDARY: " << (int)i_secondary
				<< " DIFF: " << diff
				<< " RESULT: " << result << endl;
	return result;
}

int	main()
{
	cout << "SIZE: " << sizeof(P_Iterator) << endl;
	for (int i = 50; i > -50; --i)
	{
		check_iterator(INT8_MIN - i + 3, INT8_MAX - i);
	}
	return 0;
}