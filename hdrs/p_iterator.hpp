/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 13:35:25 by blackrider        #+#    #+#             */
<<<<<<< Updated upstream
/*   Updated: 2025/07/14 22:10:22 by Pablo Escob      ###   ########.fr       */
=======
/*   Updated: 2025/07/15 08:53:54 by blackrider       ###   ########.fr       */
>>>>>>> Stashed changes
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
		static const int16_t	ITERATOR_NO_VALID = -2;
		static const int16_t	ITERATOR_VALID_FIRST = 0;
		P_Iterator(int16_t i = ITERATOR_NO_VALID);
		P_Iterator&	operator++();
		P_Iterator&	operator++(int);
		P_Iterator&	operator+=(const int16_t incr_val);
		inline operator int16_t() const { return iterator; }
		inline int16_t	get_iterator() const { return iterator; }
		bool		update_iterator(const int16_t i_can);
		static bool check_iterators(int16_t i_primary, int16_t i_secondary);
		template <typename data_t>
		static inline data_t	get_diff(const data_t i_primary, const data_t i_secondary)
		{
			return static_cast<int64_t>(i_primary - i_secondary);
		}
		// TEST PURPOSES ONLY
		inline void	set_iterator(int16_t i)
		{
			iterator = i;
		}
	private:
		int16_t		iterator;
		static inline int16_t	check_no_valid_iterator(int16_t i)
		{
			if (i < 0 && i >= ITERATOR_NO_VALID) { return ITERATOR_VALID_FIRST; }
				return i;
		}
};

#endif // P_ITERATOR_HPP