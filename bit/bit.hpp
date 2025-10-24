/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bit.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 09:54:52 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/24 10:01:47 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Bit
{
public:

	template<typename data_t>
	static inline void set(data_t& data, const int bit)
	{
		if (bit < sizeof(data_t) * 8)
		{
			data |= (1 << bit);
		}
	}

	template <typename data_t>
	static inline void clear(data_t& data, const int bit)
	{
		if (bit < sizeof(data_t) * 8)
		{
			data &= ~(1 << bit);
		}
	}

	template <typename data_t>
	static inline bool test(const data_t data, const int bit)
	{
		if (bit < sizeof(data_t) * 8)
		{
			return (data & (1 << bit));
		}
		return false;
	}
};
