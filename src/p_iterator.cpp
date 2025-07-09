/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 14:02:34 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 14:28:20 by blackrider       ###   ########.fr       */
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
	bool result = false;

	if (i_primary > i_secondary + 1)
	{
		result = true;
	}
	if (i_primary < 0 && i_secondary > 0 && i_primary - i_secondary > 1)
	{
		result = true;
	}
	return result;
}
