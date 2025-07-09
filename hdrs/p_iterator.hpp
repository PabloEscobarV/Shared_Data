/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 13:35:25 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/09 21:57:31 by Pablo Escob      ###   ########.fr       */
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
		bool		check_iterator(const int16_t i_primary, const int16_t i_secondary) const;
	private:
		int16_t		iterator;
};
