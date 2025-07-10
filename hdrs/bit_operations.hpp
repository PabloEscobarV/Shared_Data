/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bit_operations.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 08:07:49 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/10 08:33:16 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>

#define BYTE_SIZE	8

static void set_bit(uint8_t *data, uint16_t bit, bool state)
{
	if (state)
	{
		*data |= 1 << (bit % BYTE_SIZE);
	}
	else
	{
		*data &= ~(1 << (bit % BYTE_SIZE));
	}
}

static bool	get_bit(uint8_t data, uint16_t bit)
{
	return data & (1 << bit % BYTE_SIZE);
}
