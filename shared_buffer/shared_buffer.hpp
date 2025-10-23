/*******************************************************************************************************************
 *  @file shared_buffer.hpp
 *  @brief Header file for Shared_buffer class managing a fixed-size byte buffer.
 *
 *  @date Created: 2025/07/10 08:54:29
 *  @date Updated: 2025/07/21 14:26:02
 *
 *  @par Copyright (c) 2025 ComAp a.s  All rights reserved.
 *******************************************************************************************************************/

#ifndef SHARED_BUFFER_HPP
#define SHARED_BUFFER_HPP

#include "csl_cmp_int.hpp"                      // csl_cmp_int – integer limits and reserved values
#include "csl_new.hpp"                          // csl_new_assert
#include "iallocator.hpp"                       // IAllocator – memory allocator interface

// standard library includes
#include <cstdint>
#include <cstring>                              // memcpy

namespace comap
{
/*********************************************************************************************************************
 *  @class Shared_buffer
 *  @brief Fixed-size byte buffer management.
 *
 *  @tparam count Number of shared parameters to manage.
 ********************************************************************************************************************/
class Shared_buffer
{
  public:
    /*******************************************************************************************************************
     *  @brief Constructor for Shared_buffer.
     *
     *  @param[in] size Size of the buffer to allocate.
     *
     *  Allocates a buffer of the specified size and initializes the offset.
     *******************************************************************************************************************/
    Shared_buffer(uint16_t size = 0) :
      buffer(nullptr),
      buffer_size(size),
      offset(0)
    {
      init(size);
    }

    /*******************************************************************************************************************
     *  @brief Check if a length can be accessed from the current offset.
     *
     *  @param[in] len Length to check.
     *
     *  @return true if the length can be accessed, false otherwise.
     *******************************************************************************************************************/
    inline bool check_access_len(const uint16_t len) const
    {
      return (offset + len <= static_cast<uint32_t>(buffer_size));
    }

    /*******************************************************************************************************************
     *  @brief Initializer for Shared_buffer.
     *
     *  @param[in] size Size of the buffer to allocate.
     *******************************************************************************************************************/
    inline void init(const uint16_t size)
    {
      if ((size != 0) && (buffer == nullptr))
      {
        buffer_size = size;
        buffer = new(comap::csl_new_assert, comap::IAllocator::get<comap::heap_CAN>()) uint8_t[size]();
      }
    }

    /*******************************************************************************************************************
     *  @brief Get available size in the buffer from current offset.
     *
     *  @return Available size in bytes.
     *******************************************************************************************************************/
    inline uint16_t get_available_size() const
    {
      return static_cast<uint16_t>(buffer_size - offset);
    }

    /*******************************************************************************************************************
     *  @brief Copy data from the buffer to the provided pointer.
     *
     *  @param[in] ptr_data Pointer to the destination where data will be copied.
     *  @param[in] len Length of data to copy.
     *
     *  @return true if data was copied successfully, false if out of bounds.
     *******************************************************************************************************************/
    inline bool read_data(uint8_t *ptr_data, const uint16_t len)
    {
      const bool result = (ptr_data != nullptr) && check_data_bounds(len);

      if (result)
      {
        (void)memcpy(ptr_data, buffer + offset, len);
      }
      return result;
    }

    /*******************************************************************************************************************
     *  @brief Get total size of the buffer.
     *
     *  @return Total buffer size in bytes.
     *******************************************************************************************************************/
    inline uint16_t get_size() const
    {
      return buffer_size;
    }

    /*******************************************************************************************************************
     *  @brief Get current offset in the buffer.
     *
     *  @return Current offset in bytes.
     *******************************************************************************************************************/
    inline uint16_t get_offset() const
    {
      return offset;
    }

    /*******************************************************************************************************************
     *  @brief Reset the offset to zero.
     *******************************************************************************************************************/
    inline void reset_offset()
    {
      offset = 0;
    }

    /*******************************************************************************************************************
     *  @brief Set the offset to a specific value.
     *
     *  @param[in] offset_ptr New offset value to set.
     *
     *  @return true if offset was set successfully, false if out of bounds.
     *******************************************************************************************************************/
    inline bool set_offset(const uint16_t offset_ptr)
    {
      if (offset_ptr < buffer_size)
      {
        offset = offset_ptr;
        return true;
      }
      return false;
    }

    /*******************************************************************************************************************
     *  @brief Write data to the buffer from the provided pointer.
     *
     *  @param[in] ptr_data Pointer to the source data to write.
     *  @param[in] len Length of data to write.
     *
     *  @return true if data was written successfully, false if out of bounds.
     *******************************************************************************************************************/
    inline bool write_data(const uint8_t *ptr_data, const uint16_t len)
    {
      const bool result = (ptr_data != nullptr) && check_data_bounds(len);

      if (result)
      {
        (void)memcpy(buffer + offset, ptr_data, len);
      }
      return result;
    }

  private:
    uint8_t *buffer;
    uint16_t buffer_size;
    uint16_t offset;

    /*******************************************************************************************************************
     *  @brief Check if a length can be accessed from a specific offset.
     *
     *  @param[in] len Length to check.
     *  @param[in,out] offset_ptr Offset to check from. If set to NOT_VALID, uses current offset.
     *
     *  @return true if the bounds are valid, false otherwise.
     * *******************************************************************************************************************/
    inline bool check_data_bounds(const uint16_t len)
    {
      return ((len > 0) && (get_available_size() >= len));
    }
};

} // namespace comap

#endif // SHARED_BUFFER_HPP
