/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:02:34 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/14 15:28:54 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

#include <iostream>

P_Iterator::P_Iterator(int16_t i) : iterator(i)
{
	
}

P_Iterator&	P_Iterator::operator++()
{
	++iterator;
	return *this;
}

P_Iterator&	P_Iterator::operator+=(const int16_t incr_val)
{
	iterator += incr_val;
	return *this;
}

P_Iterator&	P_Iterator::operator++(int)
{
	++iterator;
	return *this;
}

P_Iterator::operator int16_t() const
{
	return iterator;
}

int16_t	P_Iterator::get_iterator() const
{
	return iterator;
}

bool	P_Iterator::update_iterator(const int16_t i_can)
{
	bool result = false;

	if (get_diff(i_can, iterator) > ITER_DIFF)
	{
		mtx_out.lock();
		std::cout << "CURRENT ITERATOR: " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		mtx_out.unlock();
		iterator = i_can + 1;
		result = true;
	}
	return result;
}
