/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/16 14:24:39 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

#include <iostream>

P_Iterator::P_Iterator(int16_t i) : iterator(i)
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

bool	P_Iterator::update_iterator(const int16_t i_can)
{
	bool result = false;

	if (get_diff(i_can, iterator) > ITER_DIFF)
	{
		mtx_out.lock();
		std::cout << "CURRENT ITERATOR: " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		mtx_out.unlock();
		operator+=(i_can + 1);
		result = true;
	}
	return result;
}

