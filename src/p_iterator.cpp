/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:02:34 by blackrider        #+#    #+#             */
<<<<<<< Updated upstream
/*   Updated: 2025/07/14 21:26:03 by Pablo Escob      ###   ########.fr       */
=======
/*   Updated: 2025/07/15 08:54:31 by blackrider       ###   ########.fr       */
>>>>>>> Stashed changes
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
	P_Iterator temp(*this);
	iterator += incr_val;
	iterator = check_no_valid_iterator(iterator);
	return temp;
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

bool	P_Iterator::check_iterators(int16_t i_primary, int16_t i_secondary)
{
	return get_diff(i_primary, i_secondary) > ITER_DIFF;
}

