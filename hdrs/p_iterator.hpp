/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 13:35:25 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/10 16:11:08 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>

class	P_Iterator
{
	public:
		P_Iterator(int16_t i = 0);
		P_Iterator&	operator++();
		P_Iterator&	operator++(int);
		P_Iterator&	operator+=(const int16_t incr_val);
		operator int16_t() const;
		int16_t	get_iterator() const;
		bool		update_iterator(const int16_t i_can);
		int16_t	get_diff(const int16_t i_can) const;
	private:
		static const uint8_t	ITER_DIFF = 1;
		int16_t		iterator;
};
