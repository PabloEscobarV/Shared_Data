/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:02:34 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 21:58:51 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

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

	if (check_iterator(i_can, iterator))
	{
		iterator = i_can + 1;
		result = true;
	}
	return result;
}

bool	P_Iterator::check_iterator(const int16_t i_primary, const int16_t i_secondary) const
{
	return static_cast<uint16_t>(i_primary - i_secondary) > 1;
}
