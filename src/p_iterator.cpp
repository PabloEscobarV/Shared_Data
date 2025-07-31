/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 10:35:20 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/31 15:04:54 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hdrs/p_iterator.hpp"

#include <iostream>

/* see header file */
P_Iterator::P_Iterator(const uint16_t i) : iterator(i)
{

}

/* see header file */
P_Iterator& P_Iterator::operator++()
{
  return operator+=(1);
}

/* see header file */
P_Iterator& P_Iterator::operator+=(const uint16_t incr_val)
{
  // Extract lower 15 bits (remove large bit if present)
  const uint16_t tmp = iterator & (LARGE_ITER_BIT - 1u);

  // If current iterator or increment has large bit, use modulo arithmetic
  if ((iterator >= LARGE_ITER_BIT) || (incr_val >= LARGE_ITER_BIT))
  {
    // Perform addition with wrap-around in large bit range
    iterator = ((tmp + incr_val) % LARGE_ITER_BIT) | LARGE_ITER_BIT;
  }
  else
  {
    // Simple addition in normal range
    iterator += incr_val;
  }
  return *this;
}

/* see header file */
P_Iterator& P_Iterator::operator++(int)
{
  (void)0; // Suppress unused parameter warning for post-increment dummy parameter
  return operator+=(1);
}

/* see header file */
bool P_Iterator::check_left_iter_is_newer(const uint16_t i_primary, const uint16_t i_secondary)
{
  bool result = false;

  // Check large bit status for both iterators
  const bool primary_large = (i_primary >= LARGE_ITER_BIT);
  const bool secondary_large = (i_secondary >= LARGE_ITER_BIT);
  if ((primary_large) && (!secondary_large))
  {
    // Primary has large bit, secondary doesn't - primary is newer
    result = true;
  }
  else
  {
    if (((!primary_large) && (!secondary_large)) || ((primary_large) && (secondary_large)))
    {
      // Both have same large bit status, compare using difference threshold
      result = get_diff(i_primary, i_secondary) > ITER_DIFF;
    }
  }
  // Note: If secondary has large bit but primary doesn't, result remains false
  return result;
}

/* see header file */
bool P_Iterator::update_iterator(const uint16_t i_can)
{
  bool result = false;

  // Check if candidate iterator is significantly newer than current
  if (check_left_iter_is_newer(i_can, iterator))
  {
    // Update iterator to candidate + 1 (next expected sequence number)
    iterator = i_can;
    result = true;
  }
  return result;
}

/* see header file */
int16_t P_Iterator::get_diff(const uint16_t i_primary, const uint16_t i_secondary)
{
  // Calculate difference handling wrap-around correctly
  int32_t diff = static_cast<int32_t>(i_primary - i_secondary);

  // Handle wrap-around: if difference is too large positive, subtract LARGE_ITER_BIT
  if (diff >= static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff -= LARGE_ITER_BIT;
  }
  // Handle wrap-around: if difference is too large negative, add LARGE_ITER_BIT
  if (diff <= -static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff += LARGE_ITER_BIT;
  }
  return static_cast<int16_t>(diff);
}
