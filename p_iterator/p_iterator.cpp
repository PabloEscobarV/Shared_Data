/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Pablo Escobar <sataniv.rider@gmail.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/23 20:47:34 by Pablo Escob       #+#    #+#             */
/*   Updated: 2025/10/23 21:13:42 by Pablo Escob      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// own header
#include "p_iterator.hpp"

#include <cstdint>

P_Iterator::P_Iterator(const uint8_t i) : iterator(i)
{

}

P_Iterator& P_Iterator::operator++()
{
  return operator+=(1);
}

P_Iterator& P_Iterator::operator++(int)
{
  P_Iterator& temp = *this;

  (void)0;
  operator+=(1);
  return temp;
}

P_Iterator& P_Iterator::operator+=(const uint8_t incr_val)
{
  const uint8_t tmp = iterator & (LARGE_ITER_BIT - 1u);
  if ((iterator >= LARGE_ITER_BIT) || (incr_val >= LARGE_ITER_BIT))
  {
    iterator = ((tmp + incr_val) % LARGE_ITER_BIT) | LARGE_ITER_BIT;
  }
  else
  {
    iterator = static_cast<uint8_t>(iterator + incr_val);
  }
  return *this;
}

bool P_Iterator::check_left_iter_is_newer(const uint8_t i_primary, const uint8_t i_secondary)
{
  bool result = false;
  const uint8_t primary_large = static_cast<uint8_t>(i_primary >= LARGE_ITER_BIT);
  const uint8_t secondary_large = static_cast<uint8_t>(i_secondary >= LARGE_ITER_BIT);

  if ((primary_large) && (!secondary_large))
  {
    result = true;
  }
  else if (!(primary_large) && (secondary_large))
  {
    result = false;
  }
  else
  {
    result = get_diff(i_primary, i_secondary) > ITER_DIFF;
  }
  return result;
}

bool P_Iterator::update_iterator(const uint8_t i_can)
{
  bool result = false;

  if (check_left_iter_is_newer(i_can, iterator))
  {
    iterator = i_can;
    result = true;
  }
  return result;
}

int8_t P_Iterator::get_diff(const uint8_t i_primary, const uint8_t i_secondary)
{
  int32_t diff = static_cast<int32_t>(i_primary - i_secondary);

  if (diff >= static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff -= LARGE_ITER_BIT;
  }
  else if (diff <= -static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff += LARGE_ITER_BIT;
  }
  else
  {

  }
  return static_cast<int8_t>(diff);
}
