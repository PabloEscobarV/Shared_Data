/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/16 08:55:00 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"
#include "../hdrs/bit_operations.hpp"

#include <iostream>

P_Iterator::P_Iterator(int16_t i) : iterator(i)
{
	
}

P_Iterator&	P_Iterator::operator++()
{
	return operator+=(1);
}

P_Iterator&	P_Iterator::operator+=(const int16_t incr_val)
{
	iterator = (iterator & MASK_HIGHER_BYTE) | (static_cast<uint8_t>(iterator) + incr_val);
	return *this;
}

P_Iterator&	P_Iterator::operator++(int)
{
	return operator+=(1);
}

void	P_Iterator::set_iterator(int16_t i)
{
	iterator = (iterator & MASK_HIGHER_BYTE) | (static_cast<uint8_t>(i));
}

bool	P_Iterator::update_iterator(const int16_t i_can)
{
	bool result = false;

	if (!get_bit(iterator, SYNCHRONIZE_BIT))
	{
		result = update_non_sync_iterator(i_can);
	}
	else
	{
		result = update_sync_iterator(i_can);
	}
	return result;
}

bool	P_Iterator::update_non_sync_iterator(int16_t i_can)
{
	bool	result = false;

	if (get_bit(i_can, SYNCHRONIZE_BIT))
	{
		mtx_out.lock();
		std::cout << "CURRENT ITERATOR (CAN-SYNC): " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		mtx_out.unlock();
		set_iterator(i_can);
		set_bit(iterator, SYNCHRONIZE_BIT, true);
		result = true;
	}
	else
	{
		mtx_out.lock();
		std::cout << "CURRENT ITERATOR (NON-CAN-SYNC): " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		mtx_out.unlock();
		set_bit(iterator, SYNCHRONIZE_BIT, true);
	}
	return result;
}

bool	P_Iterator::update_sync_iterator(int16_t i_can)
{
	bool	result = false;

	if (get_bit(i_can, SYNCHRONIZE_BIT) && check_iterators(i_can, iterator))
	{
		mtx_out.lock();
		std::cout << "CURRENT ITERATOR (SYNCHRO): " << iterator << " NEW ITERATOR: " << i_can + 1 << std::endl;
		mtx_out.unlock();
		iterator = (iterator & MASK_HIGHER_BYTE) | (static_cast<uint8_t>(i_can) + 1);
		result = true;
	}
	return result;
}