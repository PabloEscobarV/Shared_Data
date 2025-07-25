/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/24 07:36:14 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

#include <iostream>

P_Iterator::P_Iterator(uint16_t i) : iterator(i)
{
	
}

P_Iterator&	P_Iterator::operator++()
{
	return operator+=(1);
}

P_Iterator&	P_Iterator::operator+=(const uint16_t incr_val)
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
	return *this;
}

P_Iterator&	P_Iterator::operator++(int)
{
	return operator+=(1);
}

bool	P_Iterator::check_iterators(uint16_t i_primary, uint16_t i_secondary)
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

bool	P_Iterator::update_iterator(const uint16_t i_can)
{
	bool result = false;

	if (check_iterators(i_can, iterator))
	{
		// mtx_out.lock();
		// std::cout << "CURRENT ITERATOR: " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		// mtx_out.unlock();
		// Fix: Set iterator to i_can + 1, don't add to current iterator
		iterator = i_can + 1;
		result = true;
	}
	return result;
}
