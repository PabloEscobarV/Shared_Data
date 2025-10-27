/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   csl_cmp_int.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/24 20:03:47 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/26 20:48:47 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CSL_CMP_INT_HPP
#define CSL_CMP_INT_HPP

#include "../numeric_types_limits/numeric_types_limits.hpp"
#include <cstdint>

template <typename data_t>
class csl_cmp_int : public Numeric_t_limits<data_t>
{
	public:
		static inline data_t not_valid()
		{
			if (Numeric_t_limits<data_t>::is_signed())
				return Numeric_t_limits<data_t>::min();
			return Numeric_t_limits<data_t>::max();
		}
};

#endif // CSL_CMP_INT_HPP
