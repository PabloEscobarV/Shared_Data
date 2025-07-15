/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/15 14:12:10 by blackrider       ###   ########.fr       */
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
	iterator = check_no_valid_iterator(iterator);
	return *this;
}

P_Iterator&	P_Iterator::operator+=(const int16_t incr_val)
{
	iterator += incr_val;
	iterator = check_no_valid_iterator(iterator);
	return *this;
}

P_Iterator&	P_Iterator::operator++(int)
{
	++iterator;
	iterator = check_no_valid_iterator(iterator);
	return *this;
}

bool	P_Iterator::update_iterator(const int16_t i_can)
{
	bool result = false;

	if (is_no_valid_iterator(iterator) && !is_no_valid_iterator(i_can))
	{
		result = true;
	}
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

