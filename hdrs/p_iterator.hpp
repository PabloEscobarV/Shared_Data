/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 13:35:25 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/14 16:06:45 by blackrider       ###   ########.fr       */
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
		operator int16_t() const;
		int16_t	get_iterator() const;
		bool		update_iterator(const int16_t i_can);
		
		// TEST PURPOSES ONLY
		void	set_iterator(int16_t i)
		{
			iterator = i;
		}
	private:
		int16_t		iterator;
};

template <typename data_t> 
data_t	get_diff(const data_t i_primary, const data_t i_secondary)
{
	return static_cast<int64_t>(i_primary - i_secondary);
}

#endif // P_ITERATOR_HPP