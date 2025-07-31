/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   p_iterator.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: blackrider <blackrider@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 12:03:45 by blackrider        #+#    #+#             */
/*   Updated: 2025/07/31 14:59:07 by blackrider       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef P_ITERATOR_HPP
#define P_ITERATOR_HPP

#include <cstdint>

#include <mutex>

extern std::mutex mtx_out;

/*********************************************************************************************************************
 *  @class P_Iterator
 *  @brief Iterator class for handling 16-bit unsigned integer sequence numbers with wrap-around.
 *
 *  This class provides iterator functionality for uint16_t values, correctly handling
 *  wrap-around using a "large bit" mechanism at INT16_MAX + 1 (32768). It's designed for
 *  scenarios where sequence numbers might overflow and wrap around, ensuring proper
 *  message ordering in distributed parameter synchronization systems.
 *
 *  The large bit (bit 15) is used to distinguish between "old" and "new" iterator values
 *  during wrap-around conditions, allowing for correct sequence comparison even when
 *  numeric values appear to go backwards.
 ********************************************************************************************************************/
class  P_Iterator
{
  public:
    static const uint8_t  ITER_DIFF = 1; ///< Minimum difference threshold for iterator comparison

    /*******************************************************************************************************************
     *  @brief Constructor - initializes iterator with given value.
     *
     *  @param i Initial iterator value (default: 0).
     *******************************************************************************************************************/
    P_Iterator(const uint16_t i = 0);

    /*******************************************************************************************************************
     *  @brief Pre-increment operator - increments iterator by 1.
     *
     *  Increments the iterator value by 1, handling wrap-around and large bit logic.
     *  When crossing into the large bit range, the large bit is set automatically.
     *
     *  @return Reference to this iterator after increment.
     *******************************************************************************************************************/
    P_Iterator&  operator++();

    /*******************************************************************************************************************
     *  @brief Post-increment operator - increments iterator by 1.
     *
     *  Increments the iterator value by 1, handling wrap-around and large bit logic.
     *  Note: This implementation returns a reference (not a copy) for efficiency.
     *
     *  @param dummy Unused parameter to distinguish from pre-increment operator.
     *  @return Reference to this iterator after increment.
     *******************************************************************************************************************/
    P_Iterator&  operator++(int);

    /*******************************************************************************************************************
     *  @brief Addition assignment operator - adds value to iterator.
     *
     *  Adds the specified value to the iterator, handling wrap-around and large bit logic.
     *  If the current iterator or increment value is >= LARGE_ITER_BIT, modulo arithmetic
     *  is used within the large bit range.
     *
     *  @param incr_val Value to add to the iterator.
     *  @return Reference to this iterator after addition.
     *******************************************************************************************************************/
    P_Iterator&  operator+=(const uint16_t incr_val);

    /*******************************************************************************************************************
     *  @brief Explicit conversion operator to uint16_t.
     *
     *  Allows explicit conversion of the iterator to uint16_t value.
     *  Requires explicit casting to prevent accidental conversions.
     *
     *  @return Current iterator value as uint16_t.
     *******************************************************************************************************************/
    inline operator uint16_t() const { return iterator; }

    /*******************************************************************************************************************
     *  @brief Get current iterator value.
     *
     *  @return Current iterator value as uint16_t.
     *******************************************************************************************************************/
    inline uint16_t  get_iterator() const { return iterator; }

    /*******************************************************************************************************************
     *  @brief Update iterator if candidate value is significantly newer.
     *
     *  Compares the candidate iterator value with the current iterator and updates
     *  if the candidate is determined to be "newer" based on large bit priority
     *  and difference threshold. The new iterator is set to candidate + 1.
     *
     *  @param i_can Candidate iterator value to compare against.
     *  @return true if iterator was updated, false if candidate was rejected.
     *******************************************************************************************************************/
    bool  update_iterator(const uint16_t i_can);

    /*******************************************************************************************************************
     *  @brief Static method to compare two iterator values.
     *
     *  Determines if the primary iterator is "newer" than the secondary iterator
     *  based on large bit priority and difference threshold. Large bit takes
     *  precedence: values >= LARGE_ITER_BIT are considered newer than values < LARGE_ITER_BIT.
     *
     *  @param i_primary Primary iterator value to compare.
     *  @param i_secondary Secondary iterator value to compare against.
     *  @return true if primary is significantly newer than secondary.
     *******************************************************************************************************************/
    static bool check_left_iter_is_newer(const uint16_t i_primary, const uint16_t i_secondary);

    /*******************************************************************************************************************
     *  @brief Static method to calculate difference between two iterator values.
     *
     *  Calculates the signed difference between two iterator values, handling
     *  wrap-around conditions correctly using the large bit mechanism.
     *
     *  @param i_primary Primary iterator value.
     *  @param i_secondary Secondary iterator value.
     *  @return Signed difference (primary - secondary).
     *******************************************************************************************************************/
    static int16_t get_diff(const uint16_t i_primary, const uint16_t i_secondary);

    /*******************************************************************************************************************
     *  @brief Set iterator value directly (TEST PURPOSES ONLY).
     *
     *  Direct setter for iterator value, bypassing normal update logic.
     *  This method is intended for unit testing only and should not be used
     *  in production code.
     *
     *  @param i New iterator value to set.
     *******************************************************************************************************************/
    inline void set_iterator(const uint16_t i)
    {
      iterator = i;
    }

  private:
    /*******************************************************************************************************************
     *  @brief Large bit constant for wrap-around detection.
     *
     *  This constant (32768 = INT16_MAX + 1) represents the "large bit" used for
     *  distinguishing between old and new iterator values during wrap-around.
     *  Values >= LARGE_ITER_BIT are considered to have the "large bit" set.
     *******************************************************************************************************************/
    static const uint16_t  LARGE_ITER_BIT = INT16_MAX + 1;

    uint16_t  iterator; ///< Current iterator value with large bit capability
};

#endif // P_ITERATOR_HPP