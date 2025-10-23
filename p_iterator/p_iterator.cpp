/***********************************************************************************************************************
 *  @file p_iterator.cpp
 *  @brief Implementation of P_Iterator class for message sequence synchronization.
 *
 *  This file contains the implementation of the P_Iterator class methods, including
 *  increment operators, iterator comparison logic, and wrap-around handling using
 *  the large bit mechanism. The implementation ensures correct sequence ordering
 *  even when iterator values wrap around the 16-bit boundary.
 *
 *  @par Copyright (c) 2024 ComAp a.s  All rights reserved.
 **********************************************************************************************************************/

// own header
#include "p_iterator.hpp"

// standard library includes
#include <cstdint>

namespace comap
{
  /* ================================================================================================================
   *  Macro definition
   * ============================================================================================================== */

  /* ================================================================================================================
   *  Data type definition
   * ============================================================================================================== */

  /* ================================================================================================================
   *  Private constant and variable definition
   * ============================================================================================================== */

  /* ================================================================================================================
   *  Public class methods definition
   * ============================================================================================================== */

/* see header file */
P_Iterator::P_Iterator(const uint8_t i) : iterator(i)
{

}

/* see header file */
P_Iterator& P_Iterator::operator++()
{
  return operator+=(1);
}

/* see header file */
P_Iterator& P_Iterator::operator++(int)
{
  P_Iterator& temp = *this;

  (void)0; // Suppress unused parameter warning for post-increment dummy parameter
  operator+=(1);
  return temp;
}

/* see header file */
P_Iterator& P_Iterator::operator+=(const uint8_t incr_val)
{
  // Extract lower 15 bits (remove large bit if present)
  const uint8_t tmp = iterator & (LARGE_ITER_BIT - 1u);

  // If current iterator or increment has large bit, use modulo arithmetic
  if ((iterator >= LARGE_ITER_BIT) || (incr_val >= LARGE_ITER_BIT))
  {
    // Perform addition with wrap-around in large bit range
    iterator = ((tmp + incr_val) % LARGE_ITER_BIT) | LARGE_ITER_BIT;
  }
  else
  {
    // Simple addition in normal range
    iterator = static_cast<uint8_t>(iterator + incr_val);
  }
  return *this;
}

/* see header file */
bool P_Iterator::check_left_iter_is_newer(const uint8_t i_primary, const uint8_t i_secondary)
{
  bool result = false;
  const uint8_t primary_large = static_cast<uint8_t>(i_primary >= LARGE_ITER_BIT);
  const uint8_t secondary_large = static_cast<uint8_t>(i_secondary >= LARGE_ITER_BIT);

  if ((primary_large) && (!secondary_large))
  {
    // Primary has large bit, secondary doesn't - primary is newer
    result = true;
  }
  else if (!(primary_large) && (secondary_large))
  {
    result = false; // Secondary has large bit, primary doesn't - secondary is newer
  }
  else
  {
    result = get_diff(i_primary, i_secondary) > ITER_DIFF;
  }
  return result;
}

/* see header file */
bool P_Iterator::update_iterator(const uint8_t i_can)
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
int8_t P_Iterator::get_diff(const uint8_t i_primary, const uint8_t i_secondary)
{
  // Calculate difference handling wrap-around correctly
  int32_t diff = static_cast<int32_t>(i_primary - i_secondary);

  // Handle wrap-around: if difference is too large positive, subtract LARGE_ITER_BIT
  if (diff >= static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff -= LARGE_ITER_BIT;
  }
  // Handle wrap-around: if difference is too large negative, add LARGE_ITER_BIT
  else if (diff <= -static_cast<int32_t>(LARGE_ITER_BIT / 2))
  {
    diff += LARGE_ITER_BIT;
  }
  else
  {

  }
  return static_cast<int8_t>(diff);
}

} // namespace comap
