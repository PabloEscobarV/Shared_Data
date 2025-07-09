/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 13:35:25 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 14:26:35 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>

class	P_Iterator
{
	public:
		P_Iterator(int16_t i = 0);
		P_Iterator&	operator++();
		P_Iterator&	operator+=(const int16_t incr_val);
		bool	update_iterator(const int16_t i_can);
		bool	check_iterator(const int16_t i_primary, const int16_t i_secondary) const;
	private:
		int16_t		iterator;
};
