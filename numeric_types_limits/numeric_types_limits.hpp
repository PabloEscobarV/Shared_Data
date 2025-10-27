/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   numeric_types_limits.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 20:23:33 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/26 20:48:24 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NUMERIC_TYPES_LIMITS_HPP
#define NUMERIC_TYPES_LIMITS_HPP

#include <cstdint>

template <typename data_t>
class Numeric_t_limits
{
		static const data_t BYTE_SIZE = 8;
	protected:
		static inline bool	is_signed()
		{
			return (static_cast<data_t>(-1) < static_cast<data_t>(0));
		}
	public:
		static inline data_t min()
		{
			if (is_signed())
				return static_cast<data_t>(1) << (sizeof(data_t) * BYTE_SIZE - 1);
			return static_cast<data_t>(0);
		}
		static inline data_t max()
		{
			if (is_signed())
				return static_cast<data_t>(1) << (sizeof(data_t) * BYTE_SIZE - 2);
			return static_cast<data_t>(-1);
		}
};

#endif // NUMERIC_TYPES_LIMITS_HPP