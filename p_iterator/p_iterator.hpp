/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 20:45:22 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/26 13:14:19 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef P_ITERATOR_HPP
#define P_ITERATOR_HPP

#include <stdint.h>

class  P_Iterator
{
    static const uint8_t  LARGE_ITER_BIT = INT8_MAX + 1;
    uint8_t               iterator;
  public:
    static const uint8_t  ITER_DIFF = 1;

    P_Iterator(const uint8_t i = 0);

    P_Iterator&  operator++();
    P_Iterator&  operator++(int);
    P_Iterator&  operator+=(const uint8_t incr_val);
    static bool check_left_iter_is_newer(const uint8_t i_primary, const uint8_t i_secondary);
    static int8_t get_diff(const uint8_t i_primary, const uint8_t i_secondary);
    bool  update_iterator(const uint8_t i_can);
    inline operator uint8_t() const { return iterator; }
    inline uint8_t  get_iterator() const { return iterator; }
    template<typename outdata_t, typename indata_t>
    static inline outdata_t get_simply_diff(const indata_t primary, const indata_t secondary)
    {
      return static_cast<outdata_t>(static_cast<int32_t>(primary - secondary));
    }
    inline void set_iterator(const uint8_t i)
    {
      iterator = i;
    }
};

#endif // P_ITERATOR_HPP
