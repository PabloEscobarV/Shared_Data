/***********************************************************************************************************************
 *  @file p_iterator.hpp
 *  @brief Parameter iterator for message sequence synchronization.
 *
 *  @par Copyright (c) 2024 ComAp a.s  All rights reserved.
 **********************************************************************************************************************/

#ifndef P_ITERATOR_HPP
#define P_ITERATOR_HPP

#include <stdint.h>
#include "compiler_abstract.hpp"

namespace comap
{


/*********************************************************************************************************************
 *  @class P_Iterator
 *  @brief Iterator class for handling 16-bit unsigned integer sequence numbers with wrap-around.
 *
 *  This class provides iterator functionality for uint8_t values, correctly handling
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
    P_Iterator(const uint8_t i = 0);

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
    P_Iterator&  operator+=(const uint8_t incr_val);

    /*******************************************************************************************************************
     *  @brief Explicit conversion operator to uint8_t.
     *
     *  Allows explicit conversion of the iterator to uint8_t value.
     *  Requires explicit casting to prevent accidental conversions.
     *
     *  @return Current iterator value as uint8_t.
     *******************************************************************************************************************/
    inline operator uint8_t() const { return iterator; }

    /*******************************************************************************************************************
     *  @brief Get current iterator value.
     *
     *  @return Current iterator value as uint8_t.
     *******************************************************************************************************************/
    inline uint8_t  get_iterator() const { return iterator; }

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
    bool  update_iterator(const uint8_t i_can);

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
    static bool check_left_iter_is_newer(const uint8_t i_primary, const uint8_t i_secondary);

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
    static int8_t get_diff(const uint8_t i_primary, const uint8_t i_secondary);

    /*******************************************************************************************************************
     *  @brief Static method to calculate simple difference between two values.
     *
     *  Calculates difference between two values of specified types,
     *  without any wrap-around logic. Useful for general-purpose difference
     *  calculations where wrap-around is not a concern. Use only for integer type up to int32_t
     *  (does not support int64_t and uint64_t).
     *
     *  @tparam outdata_t Type of the returned signed difference.
     *  @tparam indata_t Type of the input values.
     *  @param primary Primary value.
     *  @param secondary Secondary value.
     *
     *  @return Signed difference (primary - secondary).
     *******************************************************************************************************************/
    template<typename outdata_t, typename indata_t>
    static inline outdata_t get_simply_diff(const indata_t primary, const indata_t secondary)
    {
      return static_cast<outdata_t>(static_cast<int64_t>(primary - secondary));
    }

    /*******************************************************************************************************************
     *  @brief Set iterator value directly (TEST PURPOSES ONLY).
     *
     *  Direct setter for iterator value, bypassing normal update logic.
     *  This method is intended for unit testing only and should not be used
     *  in production code.
     *
     *  @param i New iterator value to set.
     *******************************************************************************************************************/
    inline void set_iterator(const uint8_t i)
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
    static const uint8_t  LARGE_ITER_BIT = INT8_MAX + 1;

    uint8_t  iterator; ///< Current iterator value with large bit capability
};

} // namespace comap

#endif // P_ITERATOR_HPP
