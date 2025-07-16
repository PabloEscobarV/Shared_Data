/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bit_operations.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:07:49 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/16 07:28:27 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BIT_OPERATIONS_HPP
#define BIT_OPERATIONS_HPP

#include <cstdint>

#define BYTE_SIZE	8

template <typename int_t>
void set_bit(int_t& data, uint8_t bit, bool state)
{
	if (state)
	{
		data |= static_cast<int_t>(1) << bit;
	}
	else
	{
		data &= ~(static_cast<int_t>(1) << bit);
	}
}

template <typename int_t>
bool	get_bit(const int_t data, uint8_t bit)
{
	return data & (static_cast<int_t>(1) << bit);
}

#endif // BIT_OPERATIONS_HPP