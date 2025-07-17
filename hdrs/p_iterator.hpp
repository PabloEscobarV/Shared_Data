/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:03:45 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/17 12:20:16 by blackrider       ###   ########.fr       */
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
		P_Iterator(int16_t i = 0);
		P_Iterator&	operator++();
		P_Iterator&	operator++(int);
		P_Iterator&	operator+=(const int16_t incr_val);
		inline operator int16_t() const { return iterator; }
		inline int16_t	get_iterator_num() const { return iterator & 255; }
		inline int16_t	get_iterator_num(int16_t iter) const { return iter & 255; }
		bool		update_iterator(const int16_t i_can);
		static bool check_iterators(int16_t i_primary, int16_t i_secondary);
		static inline int8_t	get_diff(const int8_t i_primary, const int8_t i_secondary)
		{
			return static_cast<int16_t>(i_primary - i_secondary);
		}
		// TEST PURPOSES ONLY
		void	set_iterator(int16_t i);
	private:
		static const uint8_t	SYNCHRONIZE_BIT = sizeof(int16_t) * 8 - 1;
		static const uint16_t	MASK_HIGHER_BYTE = 0xFF00;
		int16_t		iterator;
		bool	update_non_sync_iterator(int16_t i_can);
		bool	update_sync_iterator(int16_t i_can);
};

#endif // P_ITERATOR_HPP