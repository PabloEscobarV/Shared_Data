/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/17 12:22:06 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"
#include "../hdrs/bit_operations.hpp"

#include <iostream>
#include <unistd.h>

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

bool P_Iterator::check_iterators(int16_t i_primary, int16_t i_secondary)
{
	bool result = false;
	
	if (get_bit(i_primary, SYNCHRONIZE_BIT) && get_bit(i_secondary, SYNCHRONIZE_BIT))
	{
		result = get_diff(static_cast<int8_t>(i_primary), static_cast<int8_t>(i_secondary)) > ITER_DIFF;
	}
	return result;
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
		set_iterator(static_cast<uint8_t>(i_can));
		result = true;
	}
	set_bit(iterator, SYNCHRONIZE_BIT, true);
	// mtx_out.lock();
	// std::cout << " UPDATE NON-SYNC ITERATOR" << std::endl
	// 		<< "PID: " << getpid() << std::endl
	// 		<< " ITERATOR: " << iterator << std::endl
	// 		<< " ITERATOR CAN: " << i_can << std::endl
	// 		<< " RESULT: " << result << std::endl;
	// mtx_out.unlock();
	return result;
}

bool	P_Iterator::update_sync_iterator(int16_t i_can)
{
	bool	result = false;

		// mtx_out.lock();
		// std::cout << "-------------------------========++++ UPDATE SYNC ITERATOR ++++========------------------------" << std::endl;
		// std::cout << "UPDATE SYNC ITERATOR" << std::endl
		// 		<< "PID: " << getpid() << std::endl
		// 		<< "IS SET SYNCHRONIZE_BIT OF ITERATOR: " << int(get_bit(iterator, SYNCHRONIZE_BIT)) << std::endl
		// 		<< "ITERATOR: " << int(static_cast<uint8_t>(iterator)) << std::endl
		// 		<< "IS SET SYNCHRONIZE_BIT OF CAN ITERATOR: " << int(get_bit(i_can, SYNCHRONIZE_BIT)) << std::endl
		// 		<< "ITERATOR CAN: " << int(static_cast<uint8_t>(i_can)) << std::endl
		// 		<< "RESULT: " << result << std::endl;
		// std::cout << "-------------------------========++++ UPDATE SYNC ITERATOR END!!!! ++++========------------------------" << std::endl;
		// mtx_out.unlock();
	if (get_bit(i_can, SYNCHRONIZE_BIT) && check_iterators(i_can, iterator))
	{
		iterator = (iterator & MASK_HIGHER_BYTE) | (static_cast<uint8_t>(i_can) + 1);
		set_iterator(static_cast<uint8_t>(i_can + 1));
		result = true;
	}
	return result;
}
