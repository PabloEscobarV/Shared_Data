/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:03:45 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/20 22:34:56 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef P_ITERATOR_HPP
#define P_ITERATOR_HPP

#include <cstdint>

#include <mutex>

extern std::mutex mtx_out;

class	P_Iterator
{
	public:
		static const uint8_t	ITER_DIFF = 1;
		P_Iterator(uint16_t i = 0);
		P_Iterator&	operator++();
		P_Iterator&	operator++(int);
		P_Iterator&	operator+=(const uint16_t incr_val);
		inline operator uint16_t() const { return iterator; }
		inline uint16_t	get_iterator() const { return iterator; }
		bool		update_iterator(const uint16_t i_can);
		static bool check_iterators(uint16_t i_primary, uint16_t i_secondary);
		static inline int16_t	get_diff(const uint16_t i_primary, const uint16_t i_secondary)
		{
			// Simply calculate the difference between iterator values
			return static_cast<int32_t>(i_primary - i_secondary);
		}
		// TEST PURPOSES ONLY
		inline void	set_iterator(uint16_t i)
		{
			iterator = i;
		}
	private:
		static const uint16_t	LARGE_ITER_BIT = INT16_MAX + 1;
		uint16_t		iterator;
};

#endif // P_ITERATOR_HPP