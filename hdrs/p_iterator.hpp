/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:03:45 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/15 14:11:43 by blackrider       ###   ########.fr       */
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
		static const int16_t	ITERATOR_NO_VALID = INT16_MIN;
		static const int16_t	ITERATOR_VALID_FIRST = 0;
		P_Iterator(int16_t i = ITERATOR_NO_VALID);
		P_Iterator&	operator++();
		P_Iterator&	operator++(int);
		P_Iterator&	operator+=(const int16_t incr_val);
		inline operator int16_t() const { return iterator; }
		inline int16_t	get_iterator() const { return iterator; }
		bool		update_iterator(const int16_t i_can);
		static inline bool check_iterators(int16_t i_primary, int16_t i_secondary)
		{
			return get_diff(i_primary, i_secondary) > ITER_DIFF;
		}
		static inline int16_t	get_diff(const int16_t i_primary, const int16_t i_secondary)
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
		static inline bool is_no_valid_iterator(int16_t i)
		{
			return (i == ITERATOR_NO_VALID);
		}
		static inline int16_t	check_no_valid_iterator(int16_t i)
		{
			if (i < 0 && i >= ITERATOR_NO_VALID) { return ITERATOR_VALID_FIRST; }
				return i;
		}
};

#endif // P_ITERATOR_HPP